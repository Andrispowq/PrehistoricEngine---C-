# Prehistoric Engine C++
Prehistoric Engine is a 3D game engine under development, written in C++. It uses OpenGL and Vulkan APIs to render high-end 3D graphics. 
The engine is completely customisable after compilation with the help of configuration files, including settings like window size, FPS cap, rendering settings, the scene.
Please read this file to learn about to features of this engine, and how to use it.

The engine is feature rich, and ever-expanding. Currently implemented features include: (currently only the OpenGL render images are available)

A working 3D rendering pipeline with Physically Based Shading:
![](/images/pbr_image_1.png)

A work-in-progress Atmosphere renderer with light scattering simulations:
![](/images/atmosphere_1.png)

A functional, customisable Terrain rendering system:
![](/images/terrain.png)

A helpful debug mode, rendering everything in wireframe:
![](/images/terrain_wireframe.png)

Features to come include:
* Extending the atmosphere system with a better dynamic light scattering simulation, and a day-night cycle
* A deferred renderer, with additional rendering options, including Screen-space Ambient Occlusion, Screen-space reflections, Screen-space ray tracing with compute shaders
* Multi-sample anti-aliasing for framebuffers
* A complete scripting system with the PrehistoricScript language (completely beginner-friendly, interpreted at runtime)
* An easy to use editor system for creating scenes, and game logic
* An easy way to add game logic to the GameObjects
* Porting every feature to Vulkan, extending it's current functionalities
* Porting the engine to other platforms, including
  * Linux
  * Mac OS
  * Android, and iOS
  * and probably Xbox and PlayStation

Currently, the engine only supports Windows. If you wish to use this engine, download the whole repository, and open the OpenGL 3D Game.sln file using Visual Studio 2019. 
