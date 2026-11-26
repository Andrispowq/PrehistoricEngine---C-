#!/usr/bin/env python3
"""GPU numerical checks of production shared temporal filters using analytic textures."""
import os
import subprocess
import tempfile
from pathlib import Path

root = Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='prehistoric-temporal-gpu-') as directory:
    shader = Path(directory) / 'temporal.spv'
    subprocess.run(
        ['glslc', '--target-env=vulkan1.1', str(root / 'tests/shaders/temporal_contract.comp'), '-o', str(shader)],
        check=True)
    subprocess.run(['spirv-val', '--target-env', 'vulkan1.1', str(shader)], check=True)
    env = dict(os.environ)
    molten = Path('/usr/local/share/vulkan/icd.d/MoltenVK_icd.json')
    if molten.exists(): env.setdefault('VK_DRIVER_FILES', str(molten))
    subprocess.run([str(root / 'build/debug/bin/VulkanComputeSmoke'), str(shader)], env=env, check=True, timeout=60)
    print('PASS: 256 GPU cases, repeated after pipeline recreation: stationary/invalid blur, constant HDR, '
          'history rejection/acceptance, coherent nearest-sample motion resolve, alpha preservation, '
          'and moving-strip analytic exposure reference (absolute error < 0.06)')
