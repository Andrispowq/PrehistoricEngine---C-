#!/usr/bin/env python3
"""Build/run the production IBL baker headlessly, with Vulkan validation and GPU readback.

Requires glslc, spirv-val, a C++17 compiler, and the Vulkan SDK loader/headers.
"""
import os
import subprocess
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
engine = root / 'PrehistoricEngine/src/engine'
sdk = Path(os.environ.get('VULKAN_SDK', '/usr/local'))
with tempfile.TemporaryDirectory(prefix='vulkan-ibl-') as temporary:
    work = Path(temporary)
    binaries = []
    for source in (root / 'PrehistoricEngine/res/shaders/vulkan/ibl/ibl.comp',
                   root / 'tests/shaders/vulkan_ibl_probe.comp',
                   root / 'PrehistoricEngine/res/shaders/vulkan/ibl/convert.comp'):
        binary = work / (source.stem + '.spv')
        subprocess.run(
            ['glslc', '--target-env=vulkan1.1', '-I' + str(root / 'PrehistoricEngine/res/shaders/vulkan'), str(source),
             '-o', str(binary)], check=True)
        subprocess.run(['spirv-val', '--target-env', 'vulkan1.1', str(binary)], check=True)
        binaries.append(str(binary))
    executable = work / 'VulkanIBLSmoke'
    subprocess.run([os.environ.get('CXX', 'c++'), '-std=c++17', '-O2',
                    '-I' + str(engine), '-I' + str(sdk / 'include'),
                    '-I' + str(root / 'PrehistoricEngine/Dependencies/include'),
                    str(root / 'tests/vulkan_ibl_smoke.cpp'),
                    str(engine / 'platform/vulkan/rendering/VKEnvironmentLighting.cpp'),
                    '-L' + str(sdk / 'lib'), '-Wl,-rpath,' + str(sdk / 'lib'), '-lvulkan',
                    '-o', str(executable)], check=True)
    subprocess.run([str(executable), *binaries, str(root / 'samples/res/textures/environmentMaps/cape_hill_2k.hdr')],
                   check=True, timeout=90)
