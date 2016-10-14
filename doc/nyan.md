nyan
====

## WTF?

**nyan** is a hierarchical strongly typed key-value store with patch
functionality and inheritance


## Design idea

[openage](https://github.com/SFTtech/openage) requires a very complex data
storage to represent the hierarchy of its objects. Research and technology
affects numerous units, civilization bonuses, monk conversions and all that
with the goal to be ultimatively moddable by the community:

Current data representation formats make this nearly impossible to
accomplish. Readability problems or huge lexical overhead led us to
design a language crafted for our needs.

Enter **nyan**, which is our approach to store data in a new way.


## Design goals

Requirements:

* nyan remains a general-purpose language; nicely abstracted from *openage*
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
* Some .nyan files are shipped with a game engine
  * They describe things the engine is capable of, basically the mod api
  * That way, the engine can be sure that things exist
  * The engine can access all nyan file contents with type safety
* The nyan interpreter is written in C++
  * Parses .nyan files and adds them to the store
  * Manages all data as nyan objects


## Language features

* nyan allows easy modding
  * Data packs ship configuration data and game content as .nyan files
  * Mod Packs can change and extend existing information easily,
    by applying data "patches"
  * Patches are applied whenever the libnyan user decides
    when or where to do so
* nyan is typesafe
  * The type of a member is stored when declaring it
  * No member type casts
  * Only allowed operators for a member type can be called
* nyan is invented here™
  * we can change the specification to our needs whenever we want

Concept:

* The only things nyan can do: Hierarchical data declaration and patches
  * **NyanObject**: In a .nyan file, you write down *NyanObject*s
  * Abstract *NyanObject*: has undefined members
  * Non-abstract: all members of the object have a defined value
    and e.g. a unit on screen can be created from it
* *NyanObject*s support a hierarchy by inheritance
  * You can fetch values from a *NyanObject* and the result is determined
    by walking up the whole inheritance tree
  * This allows changing a value in a parent class and all childs are
    affected then
* *NyanObject*s are placed in namespaces to organize the directory structure


### Data handling

* **NyanObject**: versatile atomic base type
  * Has named members which have a type and maybe a value
  * *NyanObject*s remain abstract until all members have values
  * There exists no order of members
* **NyanPatch**: is a *NyanObject* and denominates a patch
  * It is created for exactly one *NyanObject*,
    stored in a member named `__patch__`
  * Can modify **member values** of the assigned *NyanObject*
  * Can add **inheritance** parents of the target *NyanObject*
  * Can add new members, but not remove them
* A *NyanObject* can inherit from an ordered set of *NyanObject*s
  (-> from a *NyanPatch* as well)
  * Members of parent objects are inherited
  * When inheriting, existing values can be modified by operators
    defined for the member type
  * Member values are calculated accross the inheritance upwards
    * That way, patching a parent object impacts all children
    * When a value from a *NyanObject* is retrieved,
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
* Deep copy down feature for a *NyanObject*: copy all children.
  * This allows to copy a piece of the tree
  * This feature is required to have 2 Players with the same civilization
    that can do their research independently
  * TODO: how are objects referenced by patches then?
    * The copied object needs a new name?
    * Patch within that subtree only applies to subtree?
    * Patch from the outside applies to all subtree copies?
* A mod API could be implemented as follows:
  Create a *NyanObject* named `Mod` that has a member with a set
  of patches to apply
  * To create a mod: Inherit from this `Mod` *NyanObject* and
    add patches to the set
  * The game engine then applies the patches the appropriate way
    when a child of "Mod" is created by nyan


### Syntax

``` python
# This is an example of the nyan language
# The syntax is very much Python.
# But was enhanced to support easy hierarchical data handling.

# A NyanObject is created easily:
ObjName():
    member : TypeName = value
    ...

Inherited(ObjName, OtherObj, ...):
    member += 10

PatchName<TargetNyanObject>[+AdditionalParent, +OtherNewParent, ...]():
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
* A *NyanObject* member **type** can never be changed once declared
* The parents of a *NyanObject* are stored in a member
  `__parents__ : orderedset(NyanObject)`
  * Getting this member will provide the inheritance linearization

* It is a patch iff `<Target>` is written in the definition or the object
  has a member `__patch__ : NyanObject`
  * The patch can only be applied for the specified object or
    any child of it
  * A patch can have member `__parents_add__ : orderedset(NyanObject)`,
    the `[+AdditionalParent, ...]` syntax constructs it
  * It is used to add new objects the target should inherit from when the
    patch is applied
  * This can be used to inject a "middle object" in between two inheriting
    objects, because the multi inheritance linearization resolves the order
    * Imagine something like `TentacleMonster -> Unit`
    * What we now want is `TentacleMonster -> MonsterBase -> Unit`
    * What we do first is create `MonsterBase -> Unit`
    * What we next is patch `TentacleMonster -> Unit, MonsterBase` with `+`
    * The linearization will result in `TentacleMonster -> MonsterBase -> Unit`
* The patch will fail to be loaded if:
  * The patch target is not known
  * Any of changed members is not present in the patch target
  * Any of the added parents is not known
  * -> Blind patching is not allowed
* The patch will succeed to load if:
  * The patch target already inherits from a parent to be added
  * -> Inheritance patching doesn't conflict with other patches


#### Multi inheritance

The parents of a *NyanObject* are kind of a mixin for members:

* The child object obtains all the members from its parents
* When a member value is requested, the value is calculated by
  backtracking through all the parents until the first value definition.
* If name clashes occur, the loading will error, unless you fix them:
* Parent member names can be qualified to fix the ambiguity:

Both `Parent` and `Other` have a member named `member`:

``` python
NewObj(Parent, Other):
    Parent.member = 1337
    Other.member -= 42
```

Children of that object must access the members with the qualified names
as well to make the access clear.

Consider this case, where we have 2 conflicts.

``` python
Top():
    entry : int = 10

A(Top):
    entry += 5
    otherentry : int = 0
    specialentry : int = 42

B(Top):
    entry -= 3
    otherentry : int = 1

C():
    entry : int = 20
    otherentry : int = 2


LOLWhat(A, B, C):
    # We now have several conflicts in here!
    # How is it resolved?
    # A and B both get a member `entry` from Top
    # A and B both declare `otherentry` independently
    # C declares `entry` and `otherentry` independently
    # LOLWhat now inherits from all, so it has
    # * `entry` from Top or through A or B
    # * `entry` from C
    # * `otherentry` from A
    # * `otherentry` from B
    # * `otherentry` from C
    # ->
    # to access any of those, the name must be qualified:

    A.entry += 1     # or B.entry/Top.entry is the same!
    C.entry += 1
    A.otherentry += 1
    B.otherentry += 1
    C.otherentry += 1

    specialentry -= 42


OHNoes(LOLWhat):
    # access to qualified members remains the same
    A.entry += 1
    specialentry += 1337
```

The detection of the qualification requirement works as follows:

* The inheritance list of `LOLWhat` determined by `C3` is `[A, B, Top, C]`
* When in `LOLWhat` the `C.entry` value is requested, that list is walked
  through until a value declaration for each member was found:
  * `A` declares `otherentry` and `specialentry`, it changes `entry`
  * `B` declares `otherentry` and changes `entry`
    * Here, nyan detects that `otherentry` was declared twice
    * If it was defined without declaration, it errors because no parent
      declared `otherentry`
    * The use of `otherentry` is therefore enforced to be qualified
  * `Top` declares `entry`
  * `C` declares `entry` and `otherentry`
    * Here, nyan detects that `entry` and `otherentry` are declared again
    * The access to `entry` must hence be qualified, too
* nyan concludes that all accesses must be qualified,
  except to `specialentry`, as only one declaration was found
* The qualification is done by prefixing the precedes a *NyanObject* name
  which is somewhere up the hierarchy and would grant conflict-free access
  to that member
* That does **not** mean the value somewhere up the tree is changed!
  The change is only defined in the current object, the qualification just
  ensures the correct target member is selected!


If one now has the `OHNoes` *NyanObject* and desires to get values,
the calculation is done like this:

* Just like defining a change, the value must be queried using
  a distinct name, i. e. the qualification prefix.
* In the engine, you call something like `OHNoes.get("A.entry")`
  * The inheritance list by C3 of `OHNoes` is `[LOLWhat, A, B, Top, C]`
  * The list is gone through until the declaration of the requested member
    was found
  * `LOLWhat` did not declare it
  * `A` did not declare it either, but we requested `"A.entry"`
  * As the qualified prefix object does not declare it, the prefix is dropped
  * The member name is now unique and can be searched for without the prefix
    further up the tree
  * `B` does not declare the `entry` either
  * `Top` does declare it, now the recursion goes back the other way
  * `Top` defined the value of `entry` to `10`
  * `B` wants to subtract `3`, so `entry` is `7`
  * `A` adds `5`, so `entry` is `12`
  * `LOLWhat` adds `1`, `entry` is `13`
  * `OHNoes` adds `1` as well, and `entry` is returned to be `14`


#### Types

* Members of *NyanObject* must have a type, which can be a
  * primitive type
    - `text`:     `"lol"`          - (duh.)
    - `int`:      `1337`           - (some number)
    - `float`:    `42.235`, `inf`  - (some floating point number)
    - `bool`:     `True`, `False`  - (some boolean value)
    - `file`:     `"./name" `      - (some filename,
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
  * `bool`: `=`, `&=`, `|=`
  * `file`:`= "./delicious_cake.png"`
  * `set(type)`:
    * assignment: `= {value, value, ...}`
    * union: ` += {..}, |= {..}` -> add objects to set
    * subtract: `-= {..}` -> remove those objects
    * intersection: `&= {..}` -> keep only objects element of both
  * `orderedset(type)`:
    * assignment: `= <value, value, ...>`
    * append: `+= <..>` -> add objects to the end if not existing
    * subtract: `-= <..>, -= {..}` -> remove those objects
    * intersection: `&= <..>, &= {..}` -> keep only objects element of both
  * *NyanObject* member:
    * `=` set the reference to some other *NyanObject*
    * `@=` patch the *NyanObject* member with a compatible patch


### Namespaces and imports

Namespaces and imports work pretty much the same way as Python defined it.
They allow to organize data in an easy hierarchical way.

#### Implicit namespace

A nyan file name is implies its namespace.
A file name must not contain a `.` (except the `.nyan`) to prevent clashes.

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
* malte23 walks on your screen and dies laughing.
  It is _not_ a *NyanObject* but rather an unit object of the game engine
  which has a pointer to the `Crossbowman` *NyanObject* to fetch values from


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

A user mod that patches loom to increase villager hp by 10 instead of 15.

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
    abilities += {MonsterTeleport}

TeleportMod(Mod):
    patches = {MonsterTPPatch}
```

Why is there an `instant` member of `MoveAbility`? The game engine must
support movement without pathfinding, otherwise even movement with infinite
speed would be done by pathfinding.

This demonstrated that modding capabilities are strongly limited by the game
engine, nyan just assists you in designing a mod api in an intuitive way.
