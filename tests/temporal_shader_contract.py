#!/usr/bin/env python3
"""Compile both temporal shader interfaces and validate Vulkan SPIR-V (no GPU)."""
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SHADERS = ROOT / 'PrehistoricEngine/res/shaders'


def expand(path):
    text = path.read_text()
    return re.sub(r'#include "([^"]+)"', lambda m: expand(SHADERS / m[1]), text)


def main():
    for tool in ('glslc', 'glslangValidator', 'spirv-val', 'spirv-dis'):
        if not shutil.which(tool):
            raise SystemExit(f'Required tool missing: {tool}')
    with tempfile.TemporaryDirectory(prefix='prehistoric-temporal-shaders-') as directory:
        work = Path(directory)
        count = 0
        for source in sorted((SHADERS / 'opengl/temporal').glob('*')):
            output = work / source.name
            output.write_text(expand(source))
            subprocess.run(['glslangValidator', '-S', source.suffix[1:], str(output)], check=True)
            count += 1
        for source in sorted((SHADERS / 'vulkan/temporal').glob('*')):
            output = work / (source.name + '.spv')
            subprocess.run(['glslc', '--target-env=vulkan1.1', str(source), '-o', str(output)], check=True)
            subprocess.run(['spirv-val', '--target-env', 'vulkan1.1', str(output)], check=True)
            assembly = subprocess.check_output(['spirv-dis', str(output)], text=True)
            if source.name != 'velocity.frag' and source.name != 'fullscreen.vert':
                # std140 upload contract: a packed mat4 followed by four vec4 values.
                for member, offset in enumerate((0, 64, 80, 96, 112)):
                    assert re.search(rf'OpMemberDecorate %TemporalParameters {member} Offset {offset}\b',
                                     assembly), source
            count += 1
        print(f'PASS: {count} OpenGL/Vulkan temporal stages; Vulkan validation and uniform offsets')


if __name__ == '__main__':
    main()
