#!/usr/bin/env python3
"""Validate the compiled background shader interface and packaged HDR selection."""
import json
import re
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='vulkan-background-') as temporary:
    for stage in ('vert', 'frag', 'atmosphere'):
        source = ROOT / f'PrehistoricEngine/res/shaders/vulkan/background/background.{stage}'
        if stage == 'atmosphere':
            source = ROOT / 'PrehistoricEngine/res/shaders/vulkan/atmosphere/scattering.frag'
        binary = Path(temporary) / f'{stage}.spv'
        subprocess.run(['glslc', '--target-env=vulkan1.1', str(source), '-o', str(binary)], check=True)
        subprocess.run(['spirv-val', '--target-env', 'vulkan1.1', str(binary)], check=True)
        assembly = subprocess.check_output(['spirv-dis', str(binary)], text=True)
        if stage == 'vert':
            block = re.search(r'OpName\s+(%\S+)\s+"Background"', assembly).group(1)
            assert re.search(r'OpMemberDecorate\s+' + re.escape(block) + r'\s+0\s+Offset\s+0\b', assembly)
            assert re.search(r'OpVariable\s+%\S+\s+PushConstant', assembly)
        if stage == 'atmosphere':
            block = re.search(r'OpName\s+(%\S+)\s+"Atmosphere"', assembly).group(1)
            for member, offset in enumerate((48, 64, 80, 96, 112)):
                assert re.search(r'OpMemberDecorate\s+' + re.escape(block) + rf'\s+{member}\s+Offset\s+{offset}\b', assembly)
        if stage == 'frag':
            block = re.search(r'OpName\s+(%\S+)\s+"Background"', assembly).group(1)
            assert re.search(r'OpMemberDecorate\s+' + re.escape(block) + r'\s+0\s+Offset\s+0\b', assembly)
            sampler = re.search(r'OpName\s+(%\S+)\s+"environmentMap"', assembly).group(1)
            assert re.search(r'OpDecorate\s+' + re.escape(sampler) + r'\s+DescriptorSet\s+0\b', assembly)
            assert re.search(r'OpDecorate\s+' + re.escape(sampler) + r'\s+Binding\s+0\b', assembly)
            assert re.search(r'OpTypeImage\s+%\S+\s+Cube\b', assembly)
            assert 'OpImageSampleExplicitLod' in assembly
    hdr = ROOT / 'PrehistoricEngine/res/shaders/vulkan/postProcessing/hdr.frag'
    binary = Path(temporary) / 'hdr.spv'
    subprocess.run(['glslc', '--target-env=vulkan1.1', str(hdr), '-o', str(binary)], check=True)
    subprocess.run(['spirv-val', '--target-env', 'vulkan1.1', str(binary)], check=True)
    assembly = subprocess.check_output(['spirv-dis', str(binary)], text=True)
    for name, binding in (('scene', 0), ('bloom', 1), ('flareAtlas', 2), ('emission', 3)):
        # A helper's sampler parameter can share the descriptor's debug name.
        ids = re.findall(r'OpName\s+(%\S+)\s+"' + name + '"', assembly)
        assert any(re.search(r'OpDecorate\s+' + re.escape(id_) + rf'\s+Binding\s+{binding}\b', assembly)
                   for id_ in ids), name
    assert 'OpImageSampleImplicitLod' in assembly
    assert ' Exp ' in assembly and ' Pow ' in assembly
    for app in ('Prehistoric', 'PrehistoricEditor'):
        config = json.loads((ROOT / app / 'res/config/settings.json').read_text())
        assert config['environmentMap']['location'] == 'sample://textures/environmentMaps/cape_hill_2k.hdr'
print('PASS: background SPIR-V, camera and preview LOD push constants, HDR sampler, and shared HDR configuration')
