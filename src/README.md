# table of contents

* `main.cpp` entry point for instantiating and running the engine
* `engine/` engine code in namespace `glos`
* `application/` user code that interfaces with the engine

## rationale

* user should only write code in `application/`
* with time and applications, engine will adapt to handle encountered scenarios
* engine code is simple and can be easily adapted to the custom needs of an application

## disclaimers

* source in `hpp` files
  * unity build viewing the program as one file
  * gives compiler opportunity to optimize in a broader context
  * increases compile time and is not scalable; however, this framework is
  intended for smallish applications
* use of `static` storage and function declarations
  * gives compiler opportunity to optimize
* use of global variables
  * the engine is a singleton `glos::engine`
  * engine code is in namespace `glos`
  * most engine `hpp` files also define a global instance or instances of the
  defined type
* rather than component-based and streaming data, a monolithic approach with
shallow hierarchy is used, where object data is ordered in slices, as used by
sub-systems with cache coherence in mind
* `inline` functions
  * functions are requested to be inlined assuming compilers won't adhere to
  the hint when it does not make sense, such as big functions called from
  multiple locations
  * functions called from only one location should be inlined
* `const` is preferred and used where applicable
* `auto` is used when the type name is too verbose, such as iterators and
templates; otherwise, types are spelled out for readability
* right to left notation `Type const &inst` instead of `const Type &inst`
  * for consistency, `const` and `constexpr` are written after the type such as
  `char const *ptr` instead of `const char *ptr` and `float constexpr x` instead
  of `constexpr float x`
  * idea being that type name is an annotation that can be replaced by `auto`
* unsigned types are used where negative values don't make sense
* `++i` instead of `i++`
  * for consistency with incrementing iterators, increments and decrements are
  done in prefix
* casting such as `char(getchar())` is ok for readability
  * `reinterpret_cast` used when syntax does not allow otherwise
* members and variables are initialized for clarity although redundant
  * some exceptions regarding buffers applied
* naming convention:
  * descriptive and verbose
  * snake case
  * lower case
* use of public members in classes
  * public members are moved to private scope when not used outside the class
  * when a change to a public member triggers other actions, accessors with same
  name are written, and then member is moved to the private scope with suffix `_`
* "Plain Old C++ Object" preferred
* "Rule of None" preferred
