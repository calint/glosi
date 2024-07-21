# table of contents
* `configuration.hpp` contains constants used by engine and game
* `application.hpp` contains game logic and implements the interface to engine
  - `application_init()` called at init
  - `application_on_update_done()` called after objects have been updated and collisions resolved. may create and change state of objects
  - `application_on_render_done()` called when render frame is done. may not create or change state of objects
  - `application_free()` called when application exits (can be empty assuming resources are reclaimed by operating system)
* `objects/` contains the game objects
  - `asteroid_large`
  - `asteroid_medium`
  - `asteroid_small`
  - `bullet`
  - `fragment`
  - `power_up`
  - `ship`
  - `tetra`
  - `cube`
  - `sphere`
  - `ufo`
  - `ufo_bullet`
* `utils.hpp` utility functions used by game and game objects

## notes
* engine is multithreaded (see `configuration.hpp` setting `threaded_grid` and `threaded_update`)
* in multiplayer mode, `threaded_grid` must be `false` for clients to run in a  deterministic way
* in single player mode, updating objects and collisions resolutions can be done on available cores by turning on `threaded_grid`
* `threaded_update` enables rendering and update running in parallel, which is not thread safe but deterministic
* in smaller applications multithreaded mode might degrade performance
* occasionally it seems as if collision detection is not working because rendering is 2d of a 3d space thus objects might overlap in 2d but not in collision in 3d
* awareness of the nature of multithreaded application is recommended
