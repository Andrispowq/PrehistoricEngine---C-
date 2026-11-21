#!/usr/bin/env python3
"""Validate the compiled background shader interface and packaged HDR selection."""
import json
import re
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='vulkan-background-') as temporary:
    for stage in ('vert', 'frag'):
        source = ROOT / f'PrehistoricEngine/res/shaders/vulkan/background/background.{stage}'
        binary = Path(temporary) / f'{stage}.spv'
        subprocess.run(['glslc', '--target-env=vulkan1.1', str(source), '-o', str(binary)], check=True)
        subprocess.run(['spirv-val', '--target-env', 'vulkan1.1', str(binary)], check=True)
        assembly = subprocess.check_output(['spirv-dis', str(binary)], text=True)
        block = re.search(r'OpName\s+(%\S+)\s+"Background"', assembly).group(1)
        for member, offset in enumerate((0, 64, 68)):
            assert re.search(r'OpMemberDecorate\s+' + re.escape(block) + rf'\s+{member}\s+Offset\s+{offset}\b',
                             assembly)
        assert re.search(r'OpVariable\s+%\S+\s+PushConstant', assembly)
        if stage == 'frag':
            sampler = re.search(r'OpName\s+(%\S+)\s+"environmentMap"', assembly).group(1)
            assert re.search(r'OpDecorate\s+' + re.escape(sampler) + r'\s+DescriptorSet\s+0\b', assembly)
            assert re.search(r'OpDecorate\s+' + re.escape(sampler) + r'\s+Binding\s+0\b', assembly)
            assert 'OpImageSampleExplicitLod' in assembly
    for app in ('Prehistoric', 'PrehistoricEditor'):
        config = json.loads((ROOT / app / 'res/config/settings.json').read_text())
        assert config['environmentMap']['location'] == 'sample://textures/environmentMaps/cape_hill_2k.hdr'
print('PASS: background SPIR-V, 72-byte push constants, HDR sampler, and shared HDR configuration')
