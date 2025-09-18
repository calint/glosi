# overview

* `engine` coordinates the subsystems
* there is no explicit world class; however, namespace `glos` contains instances
of necessary objects to implement engine
* `grid` partitions space in `cells` containing `objects`
  * `object` may overlap `grid` `cells`
  * `grid` runs `update` then `resolve_collisions` pass on `cells`
  * when `threaded_grid` is `true`, the passes call `cells` in a
  non-deterministic, parallel and unsequenced way
  * `threaded_grid` must be off in multiplayer applications
  * `object` `update` is called once every frame
  * `object` `on_collision` is called once for each collision with another
  `object` in that time slice
* `object` has reference to a 3d model, `glob`, using an index in `globs`
  * has state such as `position`, `angle`, `scale`, `velocity`, `acceleration`,
  `angular_velocity` etc
* `glob`
  * `render` using opengl with a provided model to world coordinates transform matrix
  * references `materials` and `textures` using indices set at `load`
  * has bounding radius calculated at `load` and may additionally be bounded by
  a convex volume defined by `planes`
* `planes` can detect collision with spheres and other `planes`
  * collision with spheres is done by checking if distance from sphere center to
  all planes is less than radius or negative and can give false positives
  * collision with other `planes` is done by checking if any point in `planes` A
  is behind all `planes` B or vice versa
* `material` is stored in `materials` and are unique to a `glob`
* `texture` is stored in `textures` and can be shared by multiple `globs`
* `camera` describes how the world is viewed in `window`
  * contains matrix used by `engine` at render to transform world coordinates to
  screen
* `window` is a double buffer sdl2 opengl window displaying the rendered result
* `shaders` contains the opengl programs used for rendering
* `hud` is a heads-up-display rendered before the frame
* `net` and `net_server` handle single and multiplayer modes
  * synchronizes players signals
  * limits frame rate of all players to the slowest client
  * clients must run in a deterministic way thus `threaded_grid` must be off
* `sdl` handles initiation and shutdown of sdl2
* `metrics` keeps track of frame time and statistics
* `o1store` template that implements O(1) allocate and free of preallocated objects

## multithreaded engine

* configuration`threaded_update` enables update and render to run on different threads
* configuration `threaded_grid` enables `update` and `resolve_collisions` of
`grid` `cells` to run on available cores in parallel and unsequenced order
* attention is needed when objects are interacting with other objects during
`update` or `on_collision` because the objects being interacted with might be
running code on other threads
* guarantees given by engine:
  * only one thread is active at a time in an object's `update` or `on_collision`
  * collision between two objects is handled only once, considering that
  collision between same two objects can be detected on several threads if both
  objects overlap `grid` `cells`
* `threaded_update` creates data races between update and render thread on
`object` `position`, `angle`, `scale`, `glob_ix` and may be acceptable

## schematics

regarding `threaded_update` and `threaded_grid`

```text
   update thread                    render thread
   -----------------------------    ------------------------
   * clear grid cells               * wait for update thread
   * add objects to grid              ...
   * trigger render            ==>--------------------------
   * update objects in grid         * render
     cells using available cores      ...
   * resolve collisions in grid       ...
     cells using available cores      ...
   * wait for render thread           ...
     ...                              ...                              
   -----------------------------<== * trigger update
```

## globals

in order of dependency:

* metrics
* net
* net_server
* sdl
* window
* shaders
* camera
* hud
* textures
* materials
* globs
* objects
* grid

## notes

* `decouple.hpp` solves circular references between application objects and engine
