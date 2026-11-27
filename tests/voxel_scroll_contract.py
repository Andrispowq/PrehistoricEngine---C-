#!/usr/bin/env python3
"""Validate the scroll shader ABI and GL geometry/injection interfaces without a GPU."""
import json
import re
import subprocess
import tempfile
from pathlib import Path

SHADERS = Path(__file__).resolve().parents[1] / 'PrehistoricEngine/res/shaders'


def expand(path):
    return re.sub(r'^#include "([^"]+)"', lambda m: expand(SHADERS / m[1]), path.read_text(), flags=re.MULTILINE)


with tempfile.TemporaryDirectory(prefix='voxel-scroll-shaders-') as directory:
    work = Path(directory)
    stages = []
    for suffix, stage in [('VS', 'vert'), ('GS', 'geom'), ('FS', 'frag')]:
        output = work / ('geometry.' + stage)
        output.write_text(expand(SHADERS / f'opengl/voxel/voxelisation_{suffix}.glsl'))
        stages.append(output)
    subprocess.run(['glslangValidator', '-l', *stages], check=True)
    for source in ['voxel_scroll_CS.glsl', 'voxel_inject_CS.glsl', 'voxel_filter_CS.glsl', 'voxel_view_CS.glsl',
                   'terrain_CS.glsl']:
        output = work / 'shader.comp'
        output.write_text(expand(SHADERS / 'opengl/voxel' / source))
        subprocess.run(['glslangValidator', output], check=True)
    for source in ['scroll.comp', 'occupancy.comp']:
        output = work / 'shader.spv'
        subprocess.run(['glslc', '--target-env=vulkan1.1', SHADERS / 'vulkan/voxel' / source, '-o', output], check=True)
        subprocess.run(['spirv-val', '--target-env', 'vulkan1.1', output], check=True)
        interface = json.loads(subprocess.check_output(['spirv-cross', output, '--reflect']))
        block = interface['types'][interface['push_constants'][0]['type']]['members']
        assert block[-1]['offset'] == (16 if source == 'scroll.comp' else 112)
        if source == 'scroll.comp':
            assert {buffer['binding'] for buffer in interface['ssbos']} == {0, 1}
print('Voxel scroll ABI, geometry and injection shaders passed.')
