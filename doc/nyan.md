nyan
====

## WTF?

**nyan** is a hierarchical strongly typed key-value store with patch
functionality and inheritance

## Design goals

Requirements:

* nyan remains a general-purpose language; nicely abstracted from openage
* Data is stored in .nyan files
* Human readable
* Portable
* More or less compact (readability > memory)
* Easy patch-definitions to allow nyan objects to overlay other objects, the
  same mechanism is used for initial loading of all mods as well as applying
  techs
* We need patches (mod) that patch other patches (e.g. a tech for more hp).
* Does not contain any code. The game engine is responsible for calling API
  functions or redirecting to custom scripts
* Namespaces to create a logical hierarchy

* Some .nyan files are shipped with the engine
  * they describe things the engine is capable of, basically the mod api
  * that way, the engine can be sure that things exist
  * the engine can access all nyan file contents with type safety

* The nyan interpreter is written in C++
  * parses .nyan files and adds them to the store
  * manages all data as nyan objects


## Language features

* nyan is typesafe
  * Strongly typed
  * The type of a member is stored when declaring it
* nyan allows easy modding
  * Data packs ship configuration data and game content as .nyan files
  * Mod Packs can change and extend existing information easily,
    by applying data "patches"
  * Patches are applied whenever the libnyan user decides
    when or where to do so
* nyan is invented here™
  * we can change the specification to our needs whenever we want

Concept:

* The only thing that nyan can do: Data patches.
  * **NyanObject**: nyan only defines nyanobjects,
    all the time you write anything
  * Abstract *NyanObject*: has undefined members
  * Instanciable: all members have a value and e.g. a unit on screen can be
    created from it

* *NyanObject*s support a hierarchy by inheritance
* They can be placed in namespaces to organize the directory structure


### Data handling

* **NyanObject**: versatile atomic base type
  * has named members which have a type and maybe a value
  * *NyanObject*s remain abstract until all members have values
* **NyanPatch**: inherits from *NyanObject* and denominates a patch
  * Patches are created for exactly one *NyanObject*
  * Can modify member values of the assigned *NyanObject*
  * Can modify the **inheritance set** of a *NyanObject* by adding and
    removing elements
  * They can add new members, but not remove them
* A *NyanObject* can inherit from a set of *NyanObject*s
  (-> from a *NyanPatch* as well).
  * Members of parent objects are inherited
  * If there is a name clash, i. e. the same member name is defined
    by two parents, you must resolve it
    * Qualify the parent name, the inherited members then have a prefix
    * This effectively prevents the [diamond problem](https://en.wikipedia.org/wiki/Multiple_inheritance#The_diamond_problem)
      as no same-named members are allowed to inherit from
  * When inheriting, existing values can be modified by operators
    defined for the member type
  * Member values are calculated accross the inheritance upwards
    * This allows a parent object patch to impact all children
    * When a value from a *NyanObject* is retrieved,
      walk up every time and sum up the value.
* Deep copy down feature for a *NyanObject*: copy all children.
  * This allows to copy a piece of the tree
  * The copied object needs a new name
  * TODO: how are objects referenced by patches then?
    * Patch within that subtree only applies to subtree?
    * Patch from the outside applies to all subtree copies?
* A mod API could be implemented as follows:
  Create a *NyanObject* "Mod" that has a member with a set
  of patches to apply.
  * To create a mod: Inherit from this *NyanObject* and
    add patches to the set.
  * Your game then applies the patches the appropriate way
    when a child of "Mod" is seen.


### Syntax


``` python
###
This is an example of the nyan language
The syntax is very much Python.
But was enhanced to support easy data handling.
We support multi-line comments :)
###

# A NyanObject is created easily:
ObjName():
    member : TypeName = value
    ...

Inherited(ObjName, OtherObj, ...):
    member += 10

PatchName<TargetNyanObject>[+AdditionalParent, -RemovedParent, ...]():
    member_to_modify = absolute_value
    member_to_update += relative_value
    new_member : TypeName = value
```

* A member is created by *declaring* it by `member_name : type`
* A member is *defined* by `member_name = value`
* The declaration and definition can be combined:
  `member_name : type = value`
* A member can never be defined if it was not declared
* A *NyanObject* is "abstract" iff it contains at least one undefined member
* A *NyanObject* is not "abstract" iff all members are defined
* A *NyanObject* member **type** can never be changed once declared
* It is a patch iff `<Target>` is written in the definition

The parents of a *NyanObject* are kind of a mixin for members:

* The child object obtains all the members from its parents
* When a member value is requested, the value is calculated by
  backtracking through all the parents until the first value definition.
* If name clashes occur, the loading will error
* Parent member names can be qualified to fix the ambiguity:

Both `Parent` and `Other` have a member named `member`:

``` python
NewObj(Parent as name, Other as thing):
    name.member = 1337
    thing.member -= 42
```

The members of `NewObj` are then named `name.member`, and children of that
object must access the member with that name.

This is all the magic needed for creating the ultimate data language for us.


#### Types

* Members of *NyanObject* must have a type, which can be a
  * primitive type
    - `text`:     `"lol"`          - (duh.)
    - `int`:      `1337`, `inf`    - (some number)
    - `float`:    `42.235`, `inf`  - (some floating point number)
    - `bool`:     `True`, `False`  - (some boolean value)
    - `file`:     `file("./name")` - (some filename,
                                      relative to the directory
                                      the defining nyan file is located at)
  * ordered set of elements of a type: `orderedset(type)`
  * set of elements of a type: `set(type)`
  * currently, there is **no** `list(type)` specified,
    but may be added later if needed
  * *NyanObject*, to allow arbitrary hierarchies

* Type hierarchy
  * A *NyanObject*'s type name equals its name: `A()` has type `A`
  * A *NyanObject* `isinstance` of all the types of its parent *NyanObject*s
    * Sounds complicated, but is totally easy:
    * If an object `B` inherits from an object `A`, it also has the type `A`
    * Just like the multi inheritance of other programming languages
    * Again, name clashes of members must be resolved
      to avoid the diamond problem

* All members support the assignment operator `=`
* Many other operators are defined on the primitive types
  * `text`: `=`, `+=`
  * `int` and `float`: `=`, `+=`, `*=`, `-=`, `/=`
  * `set(type)`:
    * assignment: `= {value, value, ...}`
    * union: ` += {..}, |= {..}` -> add objects to set
    * subtract: `-= {..}` -> remove those objects
    * difference: `&= {..}` -> keep only objects element of both
  * `orderedset(type)`:
    * assignment: `= <value, value, ...>`
    * append: `+= <..>` -> add objects to the end if not existing
    * subtract: `-= {..}` -> remove those objects
    * difference: `&= {..}` -> keep only objects element of both
  * No operator is defined on a *NyanObject* member, except `=` of course


### Namespaces and imports

Namespaces and imports work pretty much the same way as Python defined it.
They allow to organize data in an easy hierarchical way.

#### Implicit namespace

A nyan file name is implies its namespace.

`lol/mod/component/rofl.nyan`

Data defined in the file is in namespace:

`lol.mod.component.rofl`

#### Importing

Before defining any *NyanObject*, you can import other namespaces. This leads
to the parsing of this file first, if not already loaded, just like Python
does it.

* `import openage.civs.britain`
* `import crazyguy.tentaclemod as monstermod`
* Maybe we can extend the way of importing if the need arises

You can then access the contents of that namespace in a qualified way.

`MultiheadMonster(monstermod.TentacleMonster)...`


## nyan interpreter

`.nyan` files are read by the nyan interpreter part of `libnyan`.

* You feed the .nyan files into it
* It parses the contents and adds it to the active store
* It does type checking to verify the integrity of the data hierarchy
* You can query any member and object of the store
* You can apply patches to any object at any time


## nyanc - the nyan compiler

**nyanc** can compile a .nyan file to a .h and .cpp file, this just creates
a new nyan type the same way the primitive types from above are defined.

Members can then be acessed directly from c++.


## openage specific "standard library"

nyan in openage has specific requirements how to handle patches:
mods, technologies, technology-technologies.

### Defined *NyanObject*s

The openage engine defines a few objects to inherit from.
The engine reacts differently when children of those *NyanObject*s are
created.

#### Data updates

##### `Mod`: Game mods

* It has a member `patches` where you should add your patches.
* When created, the Engine will apply the patches on load time.

##### `Tech`: Technologies

* Has a member `updates` that contains the patches to apply when researched


#### Engine features

A game engine can only process and display things it was programmed for.
That's why those features have explicit hooks when used in nyan.

The nyan definition of objects that provide configuration of such features
is thereby shipped with the engine.

A few examples

##### `Resource`: Resource types

* The engine supports adding and removing new resources via mods
* The GUI, statistics, game logic, ... subsystems dynamically take
  care of the available resources

##### `Ability`: Available unit actions

* Base object for something a unit can do
* `MoveAbility`, `GatherAbility`, ...: Defined by engine as well
* The engine implements all kinds of things for the abilities
  and also triggers actions when the ability is invoked

##### `DropSite`: Building where resources can be brought to

* The engine movement and pathfinding system must know about dropsites
* Configures the allowed resources

##### `Unit`: In-game objects

* Base object for things you can see in-game
* Provides `ability` member which contains a set of abilities

##### Many many more.

* Your game engine may define completely different objects
* How and when a patch is applied is completely up to the engine
* nyan is just the tool for keeping the data store


### Unit hierarchy

By using the objects defined by the engine, units can be defined in a
nyan file not part of the engine, but rather a data pack for it.

Lets start with an example inheritance hierarchy:

`malte23 <- Crossbowman <- Archer <- RangedUnit (engine) <- Unit (engine) <- Nyan (built in)`

Why:

* There's a base nyan objects, defined in the language
* The engine support units that move on screen
* The engine supports attack ballistics
* All archers may receive armor/attack bonus updates
* Crossbowmen is an archer and can be built at the archery
* malte23 walks on your screen and dies laughing

The only non-abstract objects can be instanciated by the game engine.
It's non-abstract when all members of an object are defined, which
is the case for a `Crossbowman`. `malte23` is instanced and handled in the
engines unit movement system.


## Modding example

### New resource

Let's create a new resource.

``` python

# Defined in the game engine:

Mod():
    name : text
    patches : set(NyanPatch)

Building():
    name : text

Resource():
    name : text
    icon : file

Gold(Resource):
    name = "Bling bling"
    icon = file("gold.svg")

Food(Resource):
    name = "Nom nom"
    icon = file("food.svg")

DropSite():
    allowed_resources : set(Resource)


# Above are engine features.
# Lets create content in your official game data pack now:

TownCenter(Building, DropSite):
    name = "Town Center"
    allowed_resources = {Gold, Food}


# Now let's have a user mod that adds a new resource:

Silicon(Resource):
    name = "Silicon"

TCSilicon<TownCenter>():
    allowed_resources += {Silicon}

SiliconMod(Mod):
    name = "The modern age has started: Behold the microchips!"
    patches = {TCSilicon}
```

When those nyan files are loaded, the data store is updated accordingly.
Your game engine implements that all `patches` from a `Mod` are applied
at game start time.

The load order of the user supplied `Mod`s is to be determined by the
game engine. Either via some mod manager, or automatic resolution.
It's up to the engine to implement.


## Patches

A patch is a nyan object that modifies another nyan object.
The patch inherits from the patched object, all unmodified
members have value None then.

### Patching a patch example

A user mod that patches loom to decrease villager hp by 10 instead of 15.

0. Loom is defined in the base data pack
1. The mod defines to update the original loom tech
2. The tech is researched, which applies the updated loom tech to the
   villager instance of the current player

``` python
# Game engine defines:
Tech():
    name : text
    updates : set(NyanPatch)

Mod():
    name : text
    patches : set(NyanPatch)

Ability():
    mouse_animation : file

Unit():
    name : text
    hp : int
    abilities : set(Ability)

Building():
    name : text
    researches : set(Tech)
    creates : set(Unit)


# Base game data defines:
Villager(Unit):
    name = "Villager"
    hp = 25

LoomVillagerHP<Villager>():
    hp += 15

Loom(Tech):
    name = "Research Loom to give villagers more HP"
    updates = {LoomVillagerHP}

TownCenter(Building):
    researches = {Loom}
    creates = {Villager}


# User mod increases the HP amount:
BalanceHP<LoomVillagerHP>():
    hp -= 5

LoomBalance(Mod):
    name = "Balance the Loom research to give"
    patches = {BalanceHP}
```

### Unit definition by mod

Let's create a new unit: a japanese tentacle monster.

```
TentacleMonster(Unit):
    name = "Splortsch"
    hp = 2000

Creation<TownCenter>():
    creates += {TentacleMonster}

TentacleMod(Mod):
    name = "Add the allmighty tentacle monster"
    patches = {Creation}
```

### Creating a new ability

Now let's create the ability to teleport for the tentacle monster
and the villager.

* should be without python code!
* ability is added as a tech to some units at runtime
  ("villagers and the tentacle monster can now teleport")
* cooldown
* maximum teleport range

```
# The engine defined:
...
MoveAbility(Ability):
    speed : float
    instant : bool = False
    range : float = inf

CooldownAbility():
    recharge_time : float


# teleport mod:
Teleport(CooldownAbility, MoveAbility):
    name = "Teleport the unit"
    speed = 0.0
    instant = True
    mouse_animation = file("arcane_wobbly.gif")

MonsterTeleport(Teleport):
    recharge_time = 30.0
    range = 5

MonsterTPPatch<TentacleMonster>():
    abilities += [MonsterTeleport]

TeleportMod(Mod):
    patches = [MonsterTPPatch]
```

Why is there a `instant` member of `MoveAbility`? The game engine must
support movement without pathfinding, otherwise even movement with infinite
speed would be done by pathfinding.
