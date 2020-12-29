# nyan

## WTF?

**nyan** is a strongly typed hierarchical key-value database with patch
functionality and inheritance.


## Design idea

[openage](https://github.com/SFTtech/openage) requires a very complex data
storage to represent the hierarchy of its objects. Research and technology
affects numerous units, civilization bonuses, monk conversions and all that
with the goal to be ultimatively moddable by the community:

Current data representation formats make this nearly impossible to
accomplish. Readability problems or huge lexical overhead led us to
design a language crafted for our needs.

Enter **nyan**, which is our approach to store data in a new way™.


## Core Principles

* Human-readable language
* More or less compact (readability > memory)
* General-purpose data definition + database features
* Changing data with patches at runtime
* Moddability of defined data
* Portable
* Object-oriented
* Typesafe


## Srsly?

Let's create a new unit with a mod: a japanese tentacle monster.

``` python
TentacleMonster(Unit):
    name = "Splortsch"
    hp = 2000

Creation<TownCenter>():
    creates += {TentacleMonster}

TentacleMod(Mod):
    name = "Add the allmighty tentacle monster to your holy army"
    patches = {Creation}
```

Things like `Unit` and `Mod` are provided by the game engine,
`TownCenter` is provided by the base game data.

When the engine activates the mod, your town center can create the new unit.


## Why nyan?

* nyan allows easy modding
  * Data packs ship configuration data and game content as `.nyan` files
  * Modpacks can change and extend existing information easily, by applying data "patches"
  * Patches are applied whenever the `libnyan` user decides when or where to do so
* nyan is typesafe
  * The type of a member is stored when declaring it
  * The only things nyan can do: Hierarchical data declaration and patches
  * No member type casts
  * Only allowed operators for a member type can be called
* nyan is invented here™
  * we can change the specification to our needs whenever we want

## Specification

A full specification is provided [here](nyan_specification.md).

## Integration into a Game Engine

* Some `.nyan` files are shipped with the game engine
  * They describe things the engine is capable of, basically the mod API
  * That way, the engine can be sure that things exist
  * The engine can access all nyan file contents with type safety
  * The data files of the game then extend and change the API `nyan::Object`s

* The nyan database provides a C++ API used by the game engine
  * Can parse `.nyan` files and add all information to the database
  * Provides hooks so the engine can react on internal changes

* Data does not contain any executable code but can specify function names
  and parameters. The game engine is responsible for calling those
  functions or redirecting to custom scripts
