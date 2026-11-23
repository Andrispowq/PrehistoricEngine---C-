#!/usr/bin/env python3
"""Link both Vulkan terrain variants and their OpenGL reference, without a GPU."""
import json
import re
import subprocess
import tempfile
from pathlib import Path

shaders = Path(__file__).resolve().parents[1] / "PrehistoricEngine/res/shaders"
terrain = shaders / "vulkan/terrain"


def expand_gl(path):
    # The engine resolves OpenGL includes against the shader resource root.
    return re.sub(r'^#include "([^"]+)"', lambda match: expand_gl(shaders / match[1]),
                  path.read_text(), flags=re.MULTILINE)


with tempfile.TemporaryDirectory(prefix="terrain-shaders-") as directory:
    work = Path(directory)
    reference = []
    for stage, suffix in (("vert", "VS"), ("tesc", "TC"), ("tese", "TE"), ("geom", "GS"), ("frag", "FS")):
        path = work / ("reference." + stage)
        path.write_text(expand_gl(shaders / f"opengl/terrain/terrain_{suffix}.glsl"))
        reference.append(path)
    subprocess.run(["glslangValidator", "-l", *reference], check=True)
    for detail in (["terrain.tese"], ["geometry.tese", "terrain.geom"]):
        stages = ["terrain.vert", "terrain.tesc", *detail, "terrain.frag"]
        subprocess.run(["glslangValidator", "-V", "--target-env", "vulkan1.1", "-l",
                        "-o", work / "linked.spv", *(terrain / stage for stage in stages)], check=True)

    for source in ("terrain.vert", "terrain.tesc", "terrain.tese", "geometry.tese", "terrain.geom",
                   "terrain.frag", "maps.comp"):
        module = work / (source + ".spv")
        subprocess.run(["glslc", "--target-env=vulkan1.1", terrain / source, "-o", module], check=True)
        subprocess.run(["spirv-val", "--target-env", "vulkan1.1", module], check=True)
        if source == "terrain.frag":
            interface = json.loads(subprocess.check_output(["spirv-cross", module, "--reflect"]))
            samplers = len(interface["textures"]) + len(interface["separate_samplers"])
            assert samplers <= 16, f"Apple M1 fragment sampler budget exceeded ({samplers})"
            assert {image["name"] for image in interface["separate_images"]} == {"terrainNormal", "terrainSplat"}
print("OpenGL reference, both Vulkan terrain variants and the terrain bake passed.")
