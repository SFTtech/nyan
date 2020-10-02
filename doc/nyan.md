nyan
----

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

* Human readable
* general-purpose data language + database features
* Portable
* Object-oriented
* Typesafe
* Changing data with patches at runtime
* Patches can be changed by patches, that way, any mod can be created


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

## Advanced Features

* nyan allows easy modding
  * Data packs ship configuration data and game content as .nyan files
  * Mod Packs can change and extend existing information easily,
    by applying data "patches"
  * Patches are applied whenever the `libnyan` user decides
    when or where to do so
* nyan is typesafe
  * The type of a member is stored when declaring it
  * No member type casts
  * Only allowed operators for a member type can be called
* nyan is invented here™
  * we can change the specification to our needs whenever we want


## Basic Structure

* Data is stored as members of `nyan::Object`s
* Data is stored in `.nyan` files
* Namespaces to create a logical hierarchy of `nyan::Object`s
* More or less compact (readability > memory)


* The only things nyan can do: Hierarchical data declaration and patches
* `nyan::Object`s support a hierarchy by inheritance
  * You can fetch values from a `nyan::Object` and the result is determined
    by walking up the whole inheritance tree
  * This allows changing a value in a parent class and all childs are
    affected then
* `nyan::Object`s are placed in namespaces to organize the directory structure

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



## Introduction

### Objects

* *`nyan::Object`*: versatile atomic base type
  * Has named members which have a type and maybe a value
  * `nyan::Object`s remain abstract until all members have values
  * There exists no order of members


### Members


### Patches

* **nyan::Patch**: is a `nyan::Object` and denominates a patch
  * Patches are used to change a target `nyan::Object` at runtime
  * It is created for exactly one `nyan::Object` with `PatchName<TargetObject>`
  * Can modify **member values** of the target `nyan::Object`
  * Can add **inheritance** parents of the target `nyan::Object`
  * Can *not* add new members or remove them
  * When activated, member values are calculated by inheritance
    * The patch inherits from the target object
    * Values are calculated top-down
    * The resulting values are stored as the target object

### Inheritance

* A `nyan::Object` can inherit from an ordered set of `nyan::Object`s
  (-> from a *nyan::Patch* as well)
  * Members of parent objects are inherited
  * When inheriting, existing values can be modified by operators
    defined for the member type
  * Member values are calculated accross the inheritance upwards
    * That way, patching a parent object impacts all children
    * When a value from a `nyan::Object` is retrieved,
      walk up every time and sum up the value
  * If there is a member name clash, there can be two reasons for it
    * The member originates from a common base object (aka the [diamond problem](https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem))
      * We use [C3 linearization](https://en.wikipedia.org/wiki/C3_linearization) to determine the calculation order
      * Just access the member as always (`member += whatever`)
    * Two independent objects define the same member
      and you inherit from both
      * The child class must access the members by `ParentObj.member`
      * Further child objects must use the same explicit access
    * If both conflicts occur simultaneously (common parent defines member
      and another parent defines it independently)
      * C3 is applied first (unifies members by a common parent)
      * Name conflicts must then resolved by manual qualification again


### Namespaces



