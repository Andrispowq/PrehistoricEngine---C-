"""Install the checksum-pinned Premake used by math CI into build/ci-tools."""
import hashlib
import io
import os
import platform
import tarfile
import urllib.request
import zipfile
from pathlib import Path

VERSION = "5.0.0-beta8"
ASSETS = {
    "Windows": ("windows.zip", "e64ce2ed8778e0098f63674cca61fe33941b5f0c8d9a4afd651152bdea3758ab"),
    "Darwin-arm64": ("macosx.tar.gz", "fa73a46f093fa6f17494a3d063421aa6cae3ea825a61c62dd59fc2f07a256d03"),
    "Darwin-x86_64": ("macosx-x64.tar.gz", "84b5fa5a432dcebdc3dd12e8677d10e38e5b32a3fe06d83ae68967e4f5e2db8a"),
}


def main():
    system = platform.system()
    key = f"Darwin-{platform.machine()}" if system == "Darwin" else system
    suffix, expected = ASSETS[key]
    url = f"https://github.com/premake/premake-core/releases/download/v{VERSION}/premake-{VERSION}-{suffix}"
    with urllib.request.urlopen(url, timeout=60) as response:
        data = response.read()
    if hashlib.sha256(data).hexdigest() != expected:
        raise RuntimeError("Premake archive checksum mismatch")
    destination = Path(__file__).resolve().parents[2] / "build" / "ci-tools"
    destination.mkdir(parents=True, exist_ok=True)
    # Extract only the executable, never archive-provided paths.
    name = "premake5.exe" if platform.system() == "Windows" else "premake5"
    if suffix.endswith(".zip"):
        with zipfile.ZipFile(io.BytesIO(data)) as archive:
            member = next(n for n in archive.namelist() if Path(n).name == name)
            executable = archive.read(member)
    else:
        with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as archive:
            member = next(m for m in archive.getmembers() if Path(m.name).name == name and m.isfile())
            executable = archive.extractfile(member).read()
    target = destination / name
    target.write_bytes(executable)
    target.chmod(0o755)
    if "GITHUB_PATH" in os.environ:
        with open(os.environ["GITHUB_PATH"], "a", encoding="utf-8") as output:
            output.write(str(destination) + "\n")
    print(target)


if __name__ == "__main__":
    main()
