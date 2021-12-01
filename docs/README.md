## Dear Py3D (prototyping)

[![build status](https://github.com/hoffstadt/DearPy3D/workflows/Windows%20Builds/badge.svg?branch=master)](https://github.com/hoffstadt/DearPy3D/actions?workflow=Windows%20Builds)

| [Goals](#goals) | [API](#api) | [Features](#features) | [Building](#building) | [Gallery](#gallery) |
|-----------------|-------------|-----------------------|-----------------------|---------------------|

As many _Dear PyGui_ users already know, we plan on adding a _Vulkan_ backend and 3D engine to [Dear PyGui](https://github.com/hoffstadt/DearPyGui).

We are still experimenting (and wrestling) with the [Vulkan](https://www.vulkan.org/) API. As we begin to feel more comfortable, we will begin adding the _Vulkan_ 
backend to _Dear PyGui_.

For now, this repo will contain the prototype of the 3D engine.

## Goals
Our goal here is **NOT** to spend too much time optimizing or cramming in every feature possible. 
The goals are:
1. Learn _Vulkan_ well.
2. Experiment with different API designs to decide what fits best within _Dear PyGui_'s API.
3. Learn/Experiment with real time rendering techniques.

## API
We are still not unsure how we want the API to look but at the moment we believe we would like to have
both a low and high level API. The question is how low and how high? 

### Low-level
How low should we go?
Should we only expose things like shaders, vertex/index buffers, textures, etc. and create a slot based resource system like D3D11?
Or should we go all the way down to descriptors, command buffers, syncronization primitives, etc. and let users build there own resource manage systems?
Where do we draw the line? Is Python performance (and the global interpreter lock or **GIL**) going to draw the line for us? Probably. 

Discuss [here](https://github.com/hoffstadt/DearPy3D/issues/13).

### High-level
We believe there is less ambiquity for the high-level interface where users will be working with items like scenes, lights, materials, meshes, shadows, animations, loading models etc.
Unlike some of the patterns for _Dear PyGui_ where we build the higher level interfaces on top of the lower level, this API will not be handled that way. This interface will
be written from the ground up to utilize more features of _Vulkan_ that we could not expose and utilize from the Python side. For example Python's **GIL** would prevent us 
from taking advantage of recording command buffers from multiple threads (not to mention **most** other performance improvements gained through multithreading).

Another benefit of creating the high level interface internally: we can also possibly support the older backends in the future(if we decide it's worth it).

Discuss [here](https://github.com/hoffstadt/DearPy3D/issues/14).

## Features
This is not an exhaustive list but contains some of the features we would like to support for the high level API (checkbox means feature is ready). Feel free to submit a PR to add on!

### Rendering
* [ ] Clustered forward renderer
* [x] Normal mapping
* [x] Mip mapping
* [ ] Sceen Space Ambient Occulusion
* [ ] Physically Based Rendering
* [x] Blinn-Phong Shading
* [ ] Image-based lighting
* [x] Point lights
* [x] Directional lights
* [ ] Spot lights
* [x] Omnidirectional Shadows
* [x] Directional Shadows
* [ ] Spot Shadows
* [ ] Cascaded Shadows
* [x] PCF Shadows
* [ ] Fog
* [ ] Clear coat
* [ ] HDR & Linear lighting
* [ ] Outlines
* [ ] Animations
* [ ] Water
* [x] Skybox
* [ ] Procedural Terrain Generation
* [ ] View Frustum Culling
* [ ] Instanced Rendering

### Post Processing
* [ ] Bloom
* [ ] MSAA
* [ ] Depth of field

### Other
* [x] OBJ file loading
* [ ] GLTF file loading

## Building
### Windows
**Requirements**
- [_git_](https://git-scm.com/)
- [_Visual Studio 2019_ (windows)](https://visualstudio.microsoft.com/vs/) with the following workflows:
  * Desktop development with C++
  * Game development with C++

Instructions:
1. From within a local directory, enter the following bash commands:
```
git clone --recursive https://github.com/hoffstadt/DearPy3D
```
4. In Visual Studio, use _Open a local folder_ to open _DearPy3D_ folder. This will use CMakeLists.txt as the "project" file.
5. Run _Build All_.

### Linux
Not ready yet.

### MacOS
Not ready yet.

## Gallery
![](https://github.com/hoffstadt/DearPyGui/blob/assets/3d_demo_3.png)
![](https://github.com/hoffstadt/DearPyGui/blob/assets/3d_demo_1.png)
![](https://github.com/hoffstadt/DearPyGui/blob/assets/3d_demo_2.png)
