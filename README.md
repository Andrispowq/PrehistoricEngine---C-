# Prehistoric Engine C++
Prehistoric Engine is an interactive application and 2D/3D game engine under development, written in C++. It uses the OpenGL and Vulkan APIs to render high-end 3D graphics. 
The engine is completely customisable after compilation with the help of configuration files, including settings like window size, FPS cap, rendering settings, the scene.

This repository not only contains the engine, but also an example project built using the engine's features.

Please read this file to learn about to features of this engine, and how to use it.

## The engine's features

A 3D renderer with Physically Based Rendering, Image Based Lighting, and many more features:
![](/images/opengl.png)

A work-in-progress Vulkan-based renderer:
![](/images/vulkan.png)

A work-in-progress Atmosphere renderer with light scattering simulations:
![](/images/atmosphere_1.png)
![](/images/atmosphere_2.png)

A work-in-progress volumetric renderer, which can currently render smoke, dust, and other effects:
![](/images/smoke_1.png)
![](/images/smoke_2.png)

A functional, customisable Terrain rendering system, paired with a helpful debug mode:
![](/images/terrain.png)
![](/images/terrain_wireframe.png)

An editor, which makes content creation easier then every before:
![](/images/editor.png)

## Planned features

* Extending the atmosphere system with a better dynamic light scattering simulation, and a day-night cycle
* Extending the renderer with animation support, hybrid rendering with ray tracing
* A complete scripting system with C#
* Improving the editor's features
* Porting every feature to Vulkan, extending it's current functionalities
* Porting the engine to other platforms, including
  * Linux
  * Mac OS
  * Android, and iOS
  * and probably Xbox and PlayStation
  
## Building the engine

The engine uses Premake 5.0 as it's build system. To learn more about using it, please click [here](BUILDING.md).

## Using the engine

If you don't want to compile the engine yourself, you should download the installer of one of the engine's releases. Please note that even the latest release could be more then 10+ commits behind the engine's current development state, so for the latest features, I recommend building the engine instead. For more information, please click [here](INSTALLING.md).
