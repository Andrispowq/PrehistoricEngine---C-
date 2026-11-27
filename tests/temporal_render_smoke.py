#!/usr/bin/env python3
"""Build a scratch app driver against the current Debug engine and run native GPU checks.
Requires an interactive desktop and the Vulkan SDK for Vulkan validation/captures.
Leaves project settings/worlds untouched. This is not a substitute for visual review.
"""
import argparse
import json
import os
import shlex
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-dir', type=Path, default=ROOT / 'build/debug')
    parser.add_argument('--api', choices=['Vulkan', 'OpenGL'], default='Vulkan')
    parser.add_argument('--app', choices=['PrehistoricEditor', 'Prehistoric'], default='PrehistoricEditor')
    parser.add_argument('--samples', type=int, default=1)
    parser.add_argument('--width', type=int, default=640)
    parser.add_argument('--height', type=int, default=360)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--world', type=Path, help='Optional diagnostic world to copy into the scratch app')
    parser.add_argument('--no-gi', action='store_true')
    parser.add_argument('--debug', type=int, choices=range(5), default=0)
    parser.add_argument('--water', action='store_true', help='Add water and exercise crossing the waterline')
    parser.add_argument('--render-controls', action='store_true', help='Exercise shared rendering keyboard shortcuts')
    parser.add_argument('--voxel-visual', action='store_true', help='Capture GI off/on and occupancy in the real scene')
    parser.add_argument('--default-scene', action='store_true', help='Keep the sample application startup scene')
    parser.add_argument('--terrain-voxels', action='store_true',
                        help='Check optional terrain voxel coverage and lifecycle')
    parser.add_argument('--world-suite', action='store_true', help='Load, round-trip and render all checked-in worlds')
    parser.add_argument('--benchmark', action='store_true',
                        help='Run four steady effect modes instead of lifecycle tests')
    parser.add_argument('--warmup', type=int, default=60, help='Warm-up frames per benchmark mode')
    parser.add_argument('--frames', type=int, default=120, help='Measured frames per benchmark mode')
    parser.add_argument('--moving-camera', action='store_true', help='Repeat a camera path in each benchmark mode')
    args = parser.parse_args()
    if sum((args.render_controls, args.terrain_voxels, args.world_suite, args.benchmark, args.voxel_visual)) > 1:
        parser.error('Choose only one smoke-test mode')
    if args.warmup < 2 or args.frames < 2:
        parser.error('--warmup and --frames must be at least 2')
    if args.benchmark and (args.water or args.debug):
        parser.error('Benchmark mode excludes lifecycle water mutations and diagnostic views')
    if args.moving_camera and not args.benchmark:
        parser.error('--moving-camera requires --benchmark')
    output = args.output or Path(tempfile.mkdtemp(prefix='prehistoric-temporal-render-'))
    output = output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    build = args.build_dir.resolve()
    original = ROOT / args.app / 'src' / ('EditorApp.cpp' if args.app == 'PrehistoricEditor' else 'PrehistoricApp.cpp')
    source = output / 'TemporalApp.cpp'
    text = original.read_text()
    driver = 'voxel_visual_driver.h' if args.voxel_visual else 'render_controls_driver.h' if args.render_controls else 'terrain_voxel_driver.h' if args.terrain_voxels else 'world_scene_driver.h' if args.world_suite else (
        'temporal_benchmark_driver.h' if args.benchmark else 'temporal_render_driver.h')
    text = (f'#include "tests/{driver}"\n'
            'namespace { const bool temporalUnbufferedOutput = [] { '
            'std::setvbuf(stdout, nullptr, _IONBF, 0); return true; }(); }\n' + text)
    if args.app == 'PrehistoricEditor':
        text = text.replace('    editor = new EditorLayer',
                            '    auto* temporalScene = scene.get();\n    editor = new EditorLayer')
        text = text.replace('    PushLayer(editor);',
                            '    PushLayer(editor);\n    PushOverlay(new TemporalRenderDriver(temporalScene));')
    else:
        text = text.replace('        return;',
                            '        PushOverlay(new TemporalRenderDriver(scene.get()));\n        return;', 1)
    if args.world_suite:
        text = text.replace('new TemporalRenderDriver(', 'new WorldSceneDriver(')
    if args.terrain_voxels:
        text = text.replace('new TemporalRenderDriver(', 'new TerrainVoxelDriver(')
    if args.render_controls:
        text = text.replace('new TemporalRenderDriver(', 'new RenderControlsDriver(')
    if args.render_controls and args.app == 'PrehistoricEditor':
        text = '#define PREHISTORIC_EDITOR_CONTROLS\n' + text
    if args.voxel_visual:
        text = text.replace('new TemporalRenderDriver(', 'new VoxelVisualDriver(')
    if args.default_scene:
        if args.app != 'Prehistoric' or not (args.render_controls or args.voxel_visual):
            parser.error('--default-scene requires Prehistoric and a controls or voxel-visual test')
        default_driver = 'VoxelVisualDriver' if args.voxel_visual else 'RenderControlsDriver'
        text = text.replace('\n}\n\nPrehistoricApp::~PrehistoricApp()',
                            f'\n    PushOverlay(new {default_driver}(scene.get()));\n}}\n\nPrehistoricApp::~PrehistoricApp()')
    source.write_text(text)
    target = f'CMakeFiles/{args.app}.dir/{original.relative_to(ROOT)}.o'
    command = subprocess.check_output(['ninja', '-C', str(build), '-t', 'commands', target], text=True).splitlines()[-1]
    compile_args = shlex.split(command)
    for flag, replacement in (('-o', str(output / 'TemporalApp.o')), ('-MF', str(output / 'TemporalApp.d'))):
        compile_args[compile_args.index(flag) + 1] = replacement
    compile_args[compile_args.index(str(original))] = str(source)
    compile_args += ['-I' + str(original.parent), '-I' + str(ROOT)]
    with (output / 'build.log').open('w') as log:
        subprocess.run(compile_args, cwd=build, stdout=log, stderr=subprocess.STDOUT, check=True)
        command = \
        subprocess.check_output(['ninja', '-C', str(build), '-t', 'commands', args.app], text=True).splitlines()[-1]
        command = command.removeprefix(': && ').removesuffix(' && :')
        link_args = shlex.split(command)
        link_args[link_args.index(target)] = str(output / 'TemporalApp.o')
        link_args[link_args.index('-o') + 1] = str(output / 'TemporalSmoke')
        subprocess.run(link_args, cwd=build, stdout=log, stderr=subprocess.STDOUT, check=True)
    res = output / 'res'
    res.mkdir(exist_ok=True)
    for entry in (ROOT / args.app / 'res').iterdir():
        target_path = res / entry.name
        if entry.name in ('config', 'world'):
            shutil.copytree(entry, target_path, dirs_exist_ok=True)
        elif target_path.exists():
            continue
        elif entry.name == 'profiling':
            target_path.mkdir()
        else:
            target_path.symlink_to(entry)
    config = res / 'config/settings.json'
    settings_data = json.loads(config.read_text())
    settings_data['framework']['api']['active'] = args.api
    settings_data['framework']['window'].update(width=args.width, height=args.height, numSamples=args.samples,
                                                maxFPS=60)
    settings_data['environmentMap'].update(environmentMapResolution=16, irradianceMapResolution=4,
                                           prefilterMapResolution=16,
                                  prefilterMapLevels=3)
    config.write_text(json.dumps(settings_data))
    world = res / 'world/sponza_midnight_gi.json'
    source_world = args.world or ROOT / args.app / 'res/world/sponza_midnight_gi.json'
    if not args.world and not source_world.exists():
        source_world = ROOT / 'PrehistoricEditor/res/world/sponza_midnight_gi.json'
    # Reusing --output must not silently reuse a diagnostic world from an earlier run.
    if source_world.resolve() != world.resolve():
        shutil.copyfile(source_world, world)
    data = json.loads(world.read_text())
    data['temporal'] = {'motion_blur': True, 'antialiasing': True, 'shutter_angle': 180, 'max_blur_pixels': 32,
                        'samples': 16}
    world.write_text(json.dumps(data))
    for capture in output.glob('*.ppm'): capture.unlink()
    env = dict(os.environ, ALSOFT_DRIVERS='null', PREHISTORIC_WORLD=str(world),
               PREHISTORIC_ENGINE_RESOURCES=str(build / 'bin/engine-resources'),
               PREHISTORIC_SAMPLE_RESOURCES=str(build / 'bin/sample-resources'))
    if args.default_scene: env.pop('PREHISTORIC_WORLD', None)
    if args.terrain_voxels or (args.render_controls and not args.world):
        flat = output / 'flat.ppm'
        flat.write_bytes(b'P6\n16 16\n255\n' + bytes([128, 128, 128]) * 256)
        terrain = json.loads((ROOT / 'Prehistoric/res/config/terrain_1.json').read_text())
        terrain['heightmap'] = str(flat)
        for material in terrain['materials']:
            for item in material['contents']:
                if item['name'] == 'heightScale': item['value'] = 0
        fixture = output / 'terrain.json'
        fixture.write_text(json.dumps(terrain))
        env['PREHISTORIC_TEST_TERRAIN_CONFIG'] = str(fixture)
        data = {'name': 'Terrain voxels', 'directories': {'models': '', 'textures': ''},
                'models': [], 'textures': [], 'materials': [], 'world_objects': [], 'root': {'children': []},
                'initial_camera': {'position': [0, 12, 20], 'forward': [0, 0.4, 1], 'up': [0, 1, 0]}}
        world.write_text(json.dumps(data))
    if args.world_suite:
        manifest = output / 'worlds.txt'
        worlds = sorted(p.resolve() for app in ('Prehistoric', 'PrehistoricEditor')
                        for p in (ROOT / app / 'res/world').rglob('*') if p.suffix in ('.json', '.wrld'))
        manifest.write_text(''.join(str(p) + '\n' for p in worlds))
        env['PREHISTORIC_WORLD_MANIFEST'] = str(manifest)
        env['PREHISTORIC_TEST_TERRAIN_CONFIG'] = str(ROOT / 'Prehistoric/res/config/terrain_1.json')
    env['PREHISTORIC_TEMPORAL_DEBUG'] = str(args.debug)
    if args.water: env['PREHISTORIC_TEMPORAL_WATER'] = '1'
    if args.no_gi: env['PREHISTORIC_TEMPORAL_NO_GI'] = '1'
    if args.benchmark:
        env['PREHISTORIC_BENCHMARK_WARMUP'] = str(args.warmup)
        env['PREHISTORIC_BENCHMARK_FRAMES'] = str(args.frames)
        if args.moving_camera: env['PREHISTORIC_BENCHMARK_MOVING'] = '1'
        settings_data['framework']['window']['vSync'] = False
        config.write_text(json.dumps(settings_data))
    if args.api == 'Vulkan':
        molten = Path('/usr/local/share/vulkan/icd.d/MoltenVK_icd.json')
        if molten.exists(): env.setdefault('VK_DRIVER_FILES', str(molten))
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_KHRONOS_validation'
        if not args.benchmark:
            env.update(VK_INSTANCE_LAYERS='VK_LAYER_KHRONOS_validation:VK_LAYER_LUNARG_screenshot',
                       VK_SCREENSHOT_FRAMES=(
                           '2,3,4,12,17,25,31,39,48,54,62' if args.world_suite else '4,12,17,25,31,39,48,54,62'),
                       VK_SCREENSHOT_DIR=str(output))
    print(f'Artifacts: {output}', flush=True)
    with (output / 'run.log').open('w') as log:
        process = subprocess.Popen([output / 'TemporalSmoke'], cwd=output, env=env, stdout=log,
                                   stderr=subprocess.STDOUT)
        try:
            code = process.wait(timeout=600 if args.world_suite else (300 if args.benchmark else 180))
        except subprocess.TimeoutExpired:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill(); process.wait()
            raise SystemExit('Timed out; see run.log for native context/window-service errors')
    log = (output / 'run.log').read_text(errors='replace')
    print(
        '\n'.join(line for line in log.splitlines() if
                  'TEMPORAL' in line or 'WORLD ' in line or 'PROCEDURAL ' in line or 'TERRAIN ' in line or 'CONTROLS ' in line or 'VOXEL VISUAL' in line))
    if code or (
            'VOXEL VISUAL PASS' if args.voxel_visual else 'CONTROLS PASS' if args.render_controls else 'TERRAIN PASS' if args.terrain_voxels else 'WORLD SUITE PASS' if args.world_suite else 'TEMPORAL PASS') not in log or 'Message severity: ERROR' in log:
        raise SystemExit(f'GPU smoke failed (exit {code}); see {output}/run.log')
    print('PASS: scripted application run, no reported Vulkan validation errors')


if __name__ == '__main__':
    main()
