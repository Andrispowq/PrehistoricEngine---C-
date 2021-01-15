# Using the engine

If you wish to use the engine, follow these steps:
- Clone the repository into a folder on your computer with the following command: `git clone --recursive https://github.com/Andrispowq/PrehistoricEngine---C-`
- Create the solution files: the build system currently supports Visual Studio (version 2017 and 2019)
  - Building the Visual Studio solution and project files: execute the `build_windows_vs2017.bat`, or `build_windows_vs2019.bat` file
  - Building for another Visual Studio version: execute the following command: `call vendor\bin\premake\premake5.exe vs<version>` from the repository's root folder (PrehistoricEngine---C-)
  - Building for another development environment: while the Prehistoric Engine is officially untested on other environments, Premake 5.0 supports environments other than Visual Studio. For the whole list, use the command `call vendor\bin\premake\premake5.exe --help` from the repository's root folder (PrehistoricEngine---C-)
- If you've followed the steps, built the project with Visual Studio, but the app won't launch (it will most likely say "The application was unable to start correctly (0xc00007b). Click OK to close the application"), you must follow these steps:
  - Navigate to the repository's root folder (PrehistoricEngine---C-), and go under `PrehistoricEngine/Dependencies/redist` and run the file called `oalinst.exe`, which will install the OpenAL 1.1 SDK, as the engine depends on the SDK being installed
