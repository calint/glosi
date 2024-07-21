# glos

experimental 3d multiplayer game framework using simple data layer 2 and opengl es3 in c++ 23

## intention
* multiplayer framework
  - synchronized client input signals
  - clients render at the rate of the slowest client
  - server synchronizes clients signals
* grid of cells that partition world space
  - update and collision detection of objects
  - multithreaded
* game where most objects are visible on screen
  - cull grid cells and objects to render within frustum _(todo)_
* importing blender exported `obj` files
* explore legacy opengl rendering
* intuitive programming of game objects and logic
* game code written in `src/application/`
* collision detection (that is "good enough" for games)
  - sphere vs sphere
  - sphere vs convex volume
  - convex volume vs convex volume
* collision reaction
  - sphere vs sphere
  - sphere vs convex volume _(todo)_
  - convex volume vs convex volume _(todo)_

## ubuntu packages used
* libgl-dev: opengl
* libglm-dev: opengl math
* libtbb-dev: parallel unsequenced `for_each`
* sdl2 related packages
* see `make.sh` for details

## howto run
* using script (recommended): `./make.sh && ./glos`
* using cmake: `mkdir build && cd build && cmake .. && make && cd .. && build/glos`

## sample game
* multiplayer enabled proof of concept
  - `threaded_grid` must be `false` for deterministic behavior
* shoot asteroids and pick up power ups for the gun
* developed to test the framework
* source in `src/application/` and `assets/`
* howto keys
  - w: thrust
  - a: rotate left
  - d: rotate right
  - j: fire
  - k, l: cameras
  - F1: print cells
  - F2: collisions on/off (this will make clients out-of-sync)
  - F3: render on/off
  - F4: cycle through shaders
  - F5: render normals on/off
  - F6: render bounding spheres on/off
  - F7: hud on/off
  - F8: render grid cells on/off

![starting multiplayer mode](https://github.com/calint/glos/assets/1920811/ed27167b-525e-4696-8a0c-2cb2fb52ec14)

![multiplayer mode](https://github.com/calint/glos/assets/1920811/697dbe55-b3b0-41ef-8bfa-ab67666291c8)

![single player mode](https://github.com/calint/glos/assets/1920811/96a27e30-d180-4f6b-930c-cf49b2c09d98)

![single player mode](https://github.com/calint/glos/assets/1920811/e39000d0-8bc9-4070-9454-46bb5ae6e4ab)

## todo
* another sample game in a 3D sandbox-like environment
* and much more in [todo.txt](https://github.com/calint/glos/blob/master/notes/todo.txt)
