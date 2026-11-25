#!/usr/bin/env python3
"""Check Vulkan PBR material layout and output encoding without a GPU.

Requires glslc, spirv-val, spirv-dis, and a C++17 compiler on PATH.
"""
import os
import re
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ENGINE = ROOT / 'PrehistoricEngine/src/engine'


def run(*args):
    return subprocess.check_output([str(a) for a in args], text=True, stderr=subprocess.STDOUT)


with tempfile.TemporaryDirectory(prefix='vulkan-pbr-contract-') as temporary:
    work = Path(temporary)
    shader = ROOT / 'PrehistoricEngine/res/shaders/vulkan/pbr/pbr.frag'
    binary = work / 'pbr.spv'
    run('glslc', '--target-env=vulkan1.1', shader, '-o', binary)
    run('spirv-val', '--target-env', 'vulkan1.1', binary)
    assembly = run('spirv-dis', binary)

    # Check the compiled interface consumed by the C++ uniform uploads.
    material = re.search(r'OpName\s+(%\S+)\s+"Material"', assembly).group(1)
    members = re.search(re.escape(material) + r'\s*=\s*OpTypeStruct\s+([^\n]+)', assembly).group(1).split()
    float_type = re.search(r'(%\S+)\s*=\s*OpTypeFloat\s+32\b', assembly).group(1)
    assert members[2] == float_type, 'Emission must be a scalar float, not RGB'
    for index, (name, offset) in enumerate((('mrot', 0), ('colour', 16), ('emission', 32), ('usesNormalMap', 48))):
        assert re.search(r'OpMemberName\s+' + re.escape(material) + rf'\s+{index}\s+"{name}"', assembly)
        assert re.search(r'OpMemberDecorate\s+' + re.escape(material) + rf'\s+{index}\s+Offset\s+{offset}\b', assembly)
    texture_set = re.findall(r'OpDecorate\s+(%\S+)\s+DescriptorSet\s+1\b', assembly)
    texture_bindings = {
        int(re.search(r'OpDecorate\s+' + re.escape(identifier) + r'\s+Binding\s+(\d+)', assembly).group(1))
        for identifier in texture_set}
    assert texture_bindings == {0, 1, 2, 3}
    assert re.search(r'OpDecorate\s+%material\s+Binding\s+3\b', assembly)
    assert 'emissionMap' not in assembly
    # IBL is global, separate from material textures. Reflect actual SPIR-V.
    for name, binding, dimension in (('irradianceMap', 3, 'Cube'), ('prefilterMap', 4, 'Cube'), ('brdfLUT', 5, '2D')):
        identifier = re.search(r'OpName\s+(%\S+)\s+"' + name + '"', assembly).group(1)
        assert re.search(r'OpDecorate\s+' + re.escape(identifier) + r'\s+DescriptorSet\s+0\b', assembly)
        assert re.search(r'OpDecorate\s+' + re.escape(identifier) + rf'\s+Binding\s+{binding}\b', assembly)
        pointer = re.search(re.escape(identifier) + r'\s*=\s*OpVariable\s+(%\S+)', assembly).group(1)
        sampled = re.search(re.escape(pointer) + r'\s*=\s*OpTypePointer\s+UniformConstant\s+(%\S+)', assembly).group(1)
        image = re.search(re.escape(sampled) + r'\s*=\s*OpTypeSampledImage\s+(%\S+)', assembly).group(1)
        assert re.search(re.escape(image) + r'\s*=\s*OpTypeImage\s+%\S+\s+' + dimension + r'\b', assembly)
    assert 'OpImageSampleExplicitLod' in assembly

    assert re.search(r'float k\s*=\s*\(\s*r\s*\*\s*r\s*\)\s*/\s*8\.0',
                     run('glslc', '-E', shader)), 'Check the analytic-light Schlick-GGX geometry term'

    # Exercise production format selection, including a simulated swapchain format
    # change. A linear 0.5 fragment must be encoded exactly once on sRGB output.
    source = work / 'encoding.cpp'
    source.write_text(r'''
#include "platform/vulkan/rendering/shaders/VKColourSpace.h"
#include <cmath>
#include <cstdio>

float srgb(float linear) {
    return linear <= 0.0031308f ? 12.92f * linear : 1.055f * std::pow(linear, 1.0f / 2.4f) - 0.055f;
}
int main() {
    for (VkFormat format : {VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB,
                            VK_FORMAT_A8B8G8R8_SRGB_PACK32}) {
        for (float configured : {1.0f, 2.2f, 2.4f}) {
            float gamma = Prehistoric::VKPBRShaderGamma(format, configured);
            if (gamma != 1.0f) return 1;
            float stored = srgb(std::pow(0.5f, 1.0f / gamma));
            if (std::abs(stored - 0.735357f) > 0.00001f) return 2;
        }
    }
    for (VkFormat format : {VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM}) {
        float gamma = Prehistoric::VKPBRShaderGamma(format, 2.2f);
        if (gamma != 2.2f || std::abs(std::pow(0.5f, 1.0f / gamma) - 0.72974f) > 0.00001f) return 3;
    }
    std::puts("Output encoding checks passed");
}
'''.replace('#include <cmath>', '#include <cmath>\n#include <initializer_list>'))
    executable = work / 'encoding'
    run(os.environ.get('CXX', 'c++'), '-std=c++17', '-I', ENGINE,
        '-I', ROOT / 'PrehistoricEngine/Dependencies/include/Vulkan', source, '-o', executable)
    run(executable)
    print('PASS: compiled PBR scalar emission layout, texture bindings, sRGB/UNORM encoding, and direct-light Schlick-GGX geometry')
