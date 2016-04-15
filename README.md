win-ray
=============================

A simple raytracer written in C++.

### Features

* Spheres, Planes, materials, lights, shadows and reflections
* Scene configuration files (INI)
* Supports Lambert diffuse and Blinn-Phong specular color
* Multi threaded (rendering is done on a separate thread to keep the window responsive)
* Parallel rendering loop with OpenMP.
* Super sampling with a variable grid size.
* sRGB
* A Camera with perspective projection and exponential exposure control
* Save images to a Targa (TGA) file

### Building

You will need Visual Studio 2013+

### Binary

A pre-built version is in the `Build` folder.
