# Using the engine

If you wish to use the engine, follow these steps:
- Clone the repository into a folder on your computer with the following command: `git clone --recursive https://github.com/Andrispowq/PrehistoricEngine---C-`
- Create the solution files: the build system currently supports Visual Studio (version 2017 and 2019)
  - Building the Visual Studio solution and project files: execute the `build_windows_vs2017.bat`, or `build_windows_vs2019.bat` file
  - Building for another Visual Studio version: execute the following command: `call vendor\bin\premake\premake5.exe vs<version>` from the repository's root folder (PrehistoricEngine---C-)
  - Building for another development environment: while the Prehistoric Engine is officially untested on other environments, Premake 5.0 supports environments other than Visual Studio. For the whole list, use the command `call vendor\bin\premake\premake5.exe --help` from the repository's root folder (PrehistoricEngine---C-)
- If you've followed the steps, built the project with Visual Studio, but the app won't launch (it will most likely say "The application was unable to start correctly (0xc00007b). Click OK to close the application"), you must follow these steps:
  - Navigate to the repository's root folder (PrehistoricEngine---C-), and go under `PrehistoricEngine/Dependencies/redist` and run the file called `oalinst.exe`, which will install the OpenAL 1.1 SDK, as the engine depends on the SDK being installed

## CLion / CMake (macOS build investigation)

Install a native Premake 5, CMake (3.21+ for the v3 presets), Ninja, and the Apple
command-line developer tools. The checked-in `premake5.exe` is Windows-only.
The generator was verified with Premake 5.0.0-beta8 and AppleClang 17 on arm64.

From the repository root:

```sh
premake5 cmake
cmake --preset debug
cmake --build --preset debug --parallel 4
```

`premake5 cmake` generates the root `CMakeLists.txt` from the existing Premake
projects. It exports all eight targets, includes, configuration-specific defines,
static-library link dependencies, and the engine precompiled header. GLFW also
gets its Cocoa sources/frameworks on macOS. Debug, Release, and Distribution
presets are supplied. CMake uses the compiler default architecture unless explicitly selected; use the
`macos-arm64` preset below on Apple Silicon, especially with an Intel CMake running
under Rosetta. The old workspace's x64 setting is not forced by the exporter.

In CLion, open the root `CMakeLists.txt` **as a project**, then select the Debug
preset/profile and allow CMake to reload. If this folder is already open as a
plain source directory, reopen it through that CMake file. The run working
directory should be `$PROJECT_DIR$/Prehistoric` or
`$PROJECT_DIR$/PrehistoricEditor`, respectively, because resource paths are relative.

The exporter is deliberately repository-specific: source membership and links
use the Debug configuration (currently identical across configurations), and
platform filters are baked for the generating host. Regenerate after changing
Premake files, adding source files, or changing host OS. Generated CMake output,
build directories, and local IDE settings are ignored by Git. The existing
Visual Studio actions remain available. The exporter does not translate arbitrary
Premake post-build commands or deployment rules; the Windows DLL copy steps are
not a macOS deployment mechanism.

**Generation/configuration succeeds; a complete macOS application build does not
yet succeed.** See [MACOS_PORT.md](MACOS_PORT.md) for verified build failures and
the source changes required to port the engine. To build the dependencies already
working on macOS:

```sh
cmake --build --preset debug --target GLFW GLAD ImGUI --parallel 4
```

## Native OpenAL on Apple Silicon

The macOS CMake path now fetches the complete upstream OpenAL Soft 1.24.2 source
release with a pinned SHA-256. Its own CMake checks generate the correct config
and build a static library with CoreAudio support. The Windows vendor snapshot
and its Premake build are retained for Windows. The first configure of each build
directory requires network access; subsequent builds reuse the downloaded source.
No global installation is needed.

```sh
premake5 cmake
cmake --preset macos-arm64
cmake --build --preset macos-arm64 --target OpenALSmoke --parallel 4
ALSOFT_DRIVERS=null ./build/macos-arm64/bin/OpenALSmoke
```

In CLion, select the `macos-arm64` CMake preset/profile and reload CMake. Build
`OpenAL` for the library alone, or `OpenALSmoke` for the standalone API/link check.
The smoke check uses the silent Null backend when run with the environment
variable above; it does not verify output through physical speakers.

The generated configuration is
`build/macos-arm64/_deps/openal_soft-build/config.h`. On Apple Silicon,
`HAVE_INTRIN_H` is undefined and the configure summary lists CoreAudio and Neon.
Do not edit this generated file or the old vendor configuration to fix macOS.
The integration lives in `scripts/native-openal.cmake` and is included by the
Premake CMake exporter. The engine and applications inherit matching upstream
OpenAL headers and static-link requirements from its target.

For an offline source checkout, pass
`-DFETCHCONTENT_SOURCE_DIR_OPENAL_SOFT=/absolute/path/to/openal-soft-1.24.2`
at configure time. The full engine still needs the other changes in MACOS_PORT.md.
