#!/usr/bin/env python3
"""Portable resource build and lookup regression check (CMake, Ninja, C++, glslc).

Runs in an isolated temporary checkout; never modifies application assets.
"""
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]


def run(*args, cwd=None, env=None, success=True):
    result = subprocess.run([str(a) for a in args], cwd=cwd, env=env,
                            text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if (result.returncode == 0) != success:
        raise AssertionError(result.stdout)
    return result.stdout


def timestamps(root):
    return {str(p.relative_to(root)): p.stat().st_mtime_ns
            for p in root.rglob('*') if p.is_file() and p.suffix != '.d'}


with tempfile.TemporaryDirectory(prefix='prehistoric resources ') as temporary:
    root = Path(temporary)
    source = root / 'source'
    shutil.copytree(REPO / 'scripts/resources', source / 'scripts/resources')
    shutil.copytree(REPO / 'PrehistoricEngine/res', source / 'PrehistoricEngine/res')
    shutil.copytree(REPO / 'samples/res', source / 'samples/res')
    build = root / 'build'
    # Configuration must explain a missing SDK/compiler, not fall back to binaries.
    isolated_env = dict(os.environ, PATH='', VULKAN_SDK=str(root / 'missing-sdk'))
    diagnostic = run(shutil.which('cmake'), '-S', source / 'scripts/resources',
                     '-B', root / 'missing-compiler', '-G', 'Ninja',
                     '-DCMAKE_MAKE_PROGRAM=' + shutil.which('ninja'),
                     '-DCMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH=FALSE',
                     '-DCMAKE_FIND_USE_CMAKE_SYSTEM_PATH=FALSE',
                     '-DCMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH=FALSE',
                     env=isolated_env, success=False)
    assert 'glslc is required' in diagnostic, diagnostic
    run('cmake', '-S', source / 'scripts/resources', '-B', build, '-G', 'Ninja')
    run('cmake', '--build', build, '--target', 'EngineResources')
    resources = build / 'bin/engine-resources'
    assert len(list(resources.rglob('*.spv'))) == 7
    assert (resources / 'shaders/vulkan/ibl/ibl_CS.spv').is_file()
    assert (resources / 'shaders/vulkan/background/background_VS.spv').is_file()
    assert (resources / 'shaders/vulkan/background/background_FS.spv').is_file()
    assert (resources / 'fonts/opensans/OpenSans-Regular.ttf').is_file()
    baseline = timestamps(resources)
    run('cmake', '--build', build, '--target', 'EngineResources')
    assert timestamps(resources) == baseline, 'No-op build changed resources'

    # Entrypoint edits rebuild only that shader; include edits rebuild the shader set.
    fragment = source / 'PrehistoricEngine/res/shaders/vulkan/pbr/pbr.frag'
    helper = fragment.parent / 'test_include.glsl'
    helper.write_text('// include dependency\n')
    original = fragment.read_text()
    run('cmake', '--build', build, '--target', 'EngineResources')
    baseline = timestamps(resources)
    fragment.write_text(original.replace('#version 450', '#version 450\n#include "test_include.glsl"'))
    run('cmake', '--build', build, '--target', 'EngineResources')
    after_source = timestamps(resources)
    changed = {k for k in baseline if baseline[k] != after_source[k]}
    assert changed == {'shaders/vulkan/pbr/pbr_FS.spv'}, changed
    helper.write_text('// changed include dependency\n')
    run('cmake', '--build', build, '--target', 'EngineResources')
    after_include = timestamps(resources)
    changed = {k for k in baseline if after_source[k] != after_include[k]}
    assert changed == {k for k in baseline if k.endswith('.spv')}, changed
    helper.write_text('this is deliberately invalid GLSL;\n')
    run('cmake', '--build', build, '--target', 'EngineResources', success=False)
    helper.write_text('// repaired\n')
    run('cmake', '--build', build, '--target', 'EngineResources')
    helper.unlink()
    run('cmake', '--build', build, '--target', 'EngineResources', success=False)
    helper.write_text('// restored\n')
    run('cmake', '--build', build, '--target', 'EngineResources')

    # Updating a sample file must not rebuild shaders or unrelated sample assets.
    sample_outputs = build / 'bin/sample-resources'
    sample_before = timestamps(sample_outputs)
    engine_before = timestamps(resources)
    material = source / 'samples/res/models/myscene.mtl'
    original_material = material.read_bytes()
    material.write_bytes(original_material + b'\n# dependency check\n')
    run('cmake', '--build', build, '--target', 'EngineResources')
    sample_after = timestamps(sample_outputs)
    assert {k for k in sample_before if sample_before[k] != sample_after[k]} == {'models/myscene.mtl'}
    assert timestamps(resources) == engine_before
    material.write_bytes(original_material)
    run('cmake', '--build', build, '--target', 'SampleResources')
    sample_before = timestamps(sample_outputs)
    run('cmake', '--build', build, '--target', 'EngineResources')
    assert timestamps(sample_outputs) == sample_before

    # Compile the actual production resolver without the rest of the engine/GPU.
    harness = root / 'harness'
    harness.mkdir()
    production = REPO / 'PrehistoricEngine/src/engine/prehistoric/core/resources'
    shutil.copy2(production / 'EngineResources.cpp', harness)
    shutil.copy2(production / 'EngineResources.h', harness)
    shutil.copy2(production / 'SharedSampleAssets.h', harness)
    (harness / 'main.cpp').write_text('''#include "EngineResources.h"
#include <iostream>
#include <exception>
int main(int argc, char** argv) {
    try {
        const std::string mode = argc > 2 ? argv[2] : "engine";
        if (mode == "resolve") std::cout << Prehistoric::ResolveAssetPath(argv[1]);
        else if (mode == "canonical") std::cout << Prehistoric::CanonicalAssetReference(argv[1]);
        else if (mode == "join") std::cout << Prehistoric::JoinAssetReference(argv[3], argv[1]);
        else std::cout << Prehistoric::EngineResourcePath(argv[1]);
        return 0;
    }
    catch (const std::exception& e) { std::cerr << e.what(); return 1; }
}
''')
    (harness / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.21)
project(ResourceLookup LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(ResourceLookup main.cpp EngineResources.cpp)
''')
    harness_build = root / 'harness-build'
    run('cmake', '-S', harness, '-B', harness_build, '-G', 'Ninja')
    run('cmake', '--build', harness_build)
    executable = harness_build / ('ResourceLookup.exe' if os.name == 'nt' else 'ResourceLookup')
    install = root / 'installed'
    run('cmake', '--install', build, '--prefix', install)
    installed_resources = install / 'engine-resources'
    assert not list(installed_resources.rglob('*.d'))
    installed_executable = install / executable.name
    shutil.copy2(executable, installed_executable)
    env = dict(os.environ)
    env.pop('PREHISTORIC_ENGINE_RESOURCES', None)
    env.pop('PREHISTORIC_SAMPLE_RESOURCES', None)
    relative = 'shaders/vulkan/pbr/pbr_FS.spv'
    assert Path(run(installed_executable, relative, cwd=root, env=env)).is_file()
    # Premake's per-application executable directory uses the common parent root.
    nested = install / 'Prehistoric'
    nested.mkdir()
    shutil.copy2(executable, nested / executable.name)
    assert Path(run(nested / executable.name, 'fonts/opensans/OpenSans-Bold.ttf', cwd=root, env=env)).is_file()
    # Both app layouts resolve the shared defaults without project-side copies.
    for source_asset in (REPO / 'PrehistoricEngine/res').rglob('*'):
        if not source_asset.is_file():
            continue
        relative_asset = source_asset.relative_to(REPO / 'PrehistoricEngine/res')
        if relative_asset.parts[0] not in ('models', 'textures'):
            continue
        resource_id = 'engine://' + relative_asset.as_posix()
        legacy_id = 'res/' + relative_asset.as_posix()
        for binary in (installed_executable, nested / executable.name):
            resolved = Path(run(binary, resource_id, 'resolve', cwd=root, env=env))
            assert resolved.read_bytes() == source_asset.read_bytes()
            assert run(binary, legacy_id, 'canonical', cwd=root, env=env) == resource_id
            assert Path(run(binary, legacy_id, 'resolve', cwd=root, env=env)) == resolved
        assert run(installed_executable, resource_id, 'join', 'res/models/', env=env) == resource_id
    assert run(installed_executable, 'res/textures/noise/Noise256_0.jpg', 'canonical',
               env=env) == 'engine://textures/noise/noise256_0.jpg'
    assert run(installed_executable, 'cube.obj', 'join', 'res/models/', env=env) == 'engine://models/cube.obj'
    assert run(installed_executable, 'res/models/quad.obj', 'join', 'res/models/',
               env=env) == 'engine://models/quad.obj'
    sample_root = REPO / 'samples/res'
    for asset in sample_root.rglob('*'):
        if not asset.is_file():
            continue
        relative_sample = asset.relative_to(sample_root).as_posix()
        resource_id = 'sample://' + relative_sample
        for binary in (installed_executable, nested / executable.name):
            resolved = Path(run(binary, resource_id, 'resolve', cwd=root, env=env))
            assert resolved.read_bytes() == asset.read_bytes()
            assert run(binary, 'res/' + relative_sample, 'canonical', env=env) == resource_id
            assert Path(run(binary, 'res/' + relative_sample, 'resolve', env=env)) == resolved
        assert run(installed_executable, resource_id, 'join', 'res/models/', env=env) == resource_id
    sample_override = dict(env, PREHISTORIC_SAMPLE_RESOURCES=str(install / 'sample-resources'))
    assert Path(run(executable, 'sample://sounds/_Closer.wav', 'resolve', env=sample_override)).is_file()
    bad_sample = dict(env, PREHISTORIC_SAMPLE_RESOURCES=str(root / 'missing-samples'))
    run(installed_executable, 'sample://models/sponza.obj', 'resolve', env=bad_sample, success=False)
    run(installed_executable, 'sample://textures/missing.png', 'resolve', env=env, success=False)
    assert run(installed_executable, 'sponza.obj', 'join', 'res/models/', env=env) == 'sample://models/sponza.obj'
    for project_asset in ('res/models/prehistoric/myscene.pmf', 'res/models/grass/grass field.obj',
                          'res/textures/water/caustics.jpg', 'res/textures/prehistoric/cape_hill_2k.prt',
                          'res/config/settings.json', 'res/world/world_1.json', 'res/scripts/CameraController.cs'):
        assert run(installed_executable, project_asset, 'resolve', cwd=root, env=env) == project_asset
    run(installed_executable, 'engine://models/missing.obj', 'resolve', cwd=root, env=env, success=False)
    override = dict(env, PREHISTORIC_ENGINE_RESOURCES=str(resources))
    assert Path(run(executable, relative, cwd=root, env=override)).is_file()
    missing = dict(env, PREHISTORIC_ENGINE_RESOURCES=str(root / 'missing'))
    assert 'Engine resource missing' in run(installed_executable, relative, cwd=root, env=missing, success=False)
    run(installed_executable, 'shaders/vulkan/missing.spv', cwd=root, env=env, success=False)
    # No source-tree fallback may silently hide a missing generated package.
    run(executable, relative, cwd=REPO / 'Prehistoric', env=env, success=False)
    print('PASS: incremental compilation, include tracking, compiler failures, install staging, '
          'executable-relative lookup, Premake layout, override and missing-resource errors')
