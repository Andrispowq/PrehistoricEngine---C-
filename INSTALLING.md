# Using the engine

If you wish to use the engine and the example project, without compiling the source code, follow these steps:
- Click on the Releases tab in the reposotary
- Under Assets, download `PrehistoricEngineSetup.exe`
- Once downloaded, run it, set the install path, and once it's finished, click Finish
- The installer will install the MSVC 2015-2019 Redistributable, but will fail to install the OpenAL 1.1 SDK (bug)
- Once everything's finished, go to the directory where you have installed the engine, go under `Prehistoric Engine/redist`, and run `oalinst.exe` in Administrator mode to install OpenAL
- Now, you can run the engine with either `Prehistoric.exe` or by using the icon created on the Desktop