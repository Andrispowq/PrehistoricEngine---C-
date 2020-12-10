# Using the engine

If you wish to use the engine, follow these steps:
- Clone the reposotary into a folder on your computer with the following command: ```git clone --recursive https://github.com/Andrispowq/PrehistoricEngine---C-```
- Create the solution files: the build system currently supports Visual Studio (version 2017 and 2019)
  - Building the Visual Studio solution and project files: execute the ```build_windows_vs2017.bat```, or ```build_windows_vs2019.bat``` file
  - Building for another Visual Studio version: execute the following command: ```call vendor\bin\premake\premake5.exe vs<version>``` from the reposotary's root folder (PrehistoricEngine)
  - Building for another development environment: while the Prehistoric Engine is officially untested on other environments, Premake 5.0 supports environments other than Visual Studio. For the whole list, use the command ```call vendor\bin\premake\premake5.exe --help``` from the reposotary's root folder (PrehistoricEngine)
