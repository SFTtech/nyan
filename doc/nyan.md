nyan
----

## WTF?

**nyan** is a strongly typed hierarchical key-value database with patch
functionality and inheritance.


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



## Design idea

[openage](https://github.com/SFTtech/openage) requires a very complex data
storage to represent the hierarchy of its objects. Research and technology
affects numerous units, civilization bonuses, monk conversions and all that
with the goal to be ultimatively moddable by the community:

Current data representation formats make this nearly impossible to
accomplish. Readability problems or huge lexical overhead led us to
design a language crafted for our needs.

Enter **nyan**, which is our approach to store data in a new way™.


## Design goals

Requirements:

* nyan remains a general-purpose data language
* Data is stored in `.nyan` files
* Human readable
* Portable
* More or less compact (readability > memory)
* Data is stored as members of `nyan::Object`s
* Data is changed by patches that change members of `nyan::Object`s
* Patches can be changed by patches, that way, any mod can be created
* Data does not contain any executed code but can specify function names
  and parameters. The game engine is responsible for calling those
  functions or redirecting to custom scripts
* Namespaces to create a logical hierarchy of `nyan::Object`s
* Some `.nyan` files are shipped with the game engine
  * They describe things the engine is capable of, basically the mod api
  * That way, the engine can be sure that things exist
  * The engine can access all nyan file contents with type safety
  * The data files of the game then extend and change the API `nyan::Object`s
* The nyan database provides a C++ API used by the game engine
  * Can parse `.nyan` files and add all information to the database
  * Provides hooks so the engine can react on internal changes


## Language features

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

Concept:

* The only things nyan can do: Hierarchical data declaration and patches
  * *`nyan::Object`*: In a .nyan file, you write down `nyan::Object`s
  * A `nyan::Object` has an aribitrary number of members
  * A member has a data type like `int`
* `nyan::Object`s support a hierarchy by inheritance
  * You can fetch values from a `nyan::Object` and the result is determined
    by walking up the whole inheritance tree
  * This allows changing a value in a parent class and all childs are
    affected then
* `nyan::Object`s are placed in namespaces to organize the directory structure


### Data handling

* *`nyan::Object`*: versatile atomic base type
  * Has named members which have a type and maybe a value
  * `nyan::Object`s remain abstract until all members have values
  * There exists no order of members
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


### Syntax

``` python
# This is an example of the nyan language
# The syntax is very much Python.
# But was enhanced to support easy hierarchical data handling.

# A nyan::Object is created easily:
ObjName():
    member : TypeName = value

    member_name : Object
    ...

Inherited(ObjName, OtherObj, ...):
    member += 10
    ObjName.member_name = "stuff"

PatchName<TargetNyanObject>[+AdditionalParent, +OtherNewParent, ...]():
    member_to_modify = absolute_value
    member_to_update += relative_value
    member_to_replace @+= relative_value
    member_to_replace_too @= absolute_value

ParentObject():
    NestedObject(Inherited):
        another_member : type = value
        ...

    some_member : Inherited = NestedObject
    ...
```

* An object declares a named storage space with then has key-value pairs
* If an object does not have a parent, it implicitly inherits from the built-in `Object`
* Nested object names are prefixed the parent name, on top level their name is `ParentObject.NestedObject`
* A member is created by *declaring* it by `member_name : type`
* A member is *defined* by `member_name = value`
* An inherited member is referenced by either `member_name` or `ParentObject.member_name`
* The declaration and definition can be combined:
  `member_name : type = value`
* A member can never be defined if it was not declared
* A `nyan::Object` is "abstract" iff it contains at least one undefined member
* A `nyan::Object` member **type** can never be changed once declared

* It is a patch iff `<Target>` is written in the definition
  * The patch will be applied for the specified object only
  * A patch can add a new inheritance parent at the front of the parent list
    * Done with the `[+AdditionalParent, AnotherParent+, ...]` syntax
    * `+Parent` adds parent to the end, `Parent+` to the front
      * Reason: the direction of the `+` indicates the existing list
      * If target has parents `[A, B]` and we apply `[+C, +D, E+, B+]`, the result is `[E, A, B, C, D]`
    * The activation of this parent must not induce name clashes of members, [see below](#Multi inheritance). When the patch is loaded, this is checked.
    * This can be used to inject a "middle object" in between two inheriting
      objects, because the multi inheritance linearization resolves the order
      * Imagine something like `TentacleMonster -> Unit`
      * What we now want is `TentacleMonster -> MonsterBase -> Unit`
      * What we do first is create `MonsterBase -> Unit`
      * After applying a patch with `+MonsterBase` it is `TentacleMonster -> MonsterBase, Unit`
      * The linearization will result in `TentacleMonster -> MonsterBase -> Unit`
  * A patch modifies the value of the target object only
    * The target object operator will remain the same
    * The target object value will be changed according to the operation
      in the patch
  * A patch replaces the operator and value of a target object,
    if the patch operation is prefixed with an `@`
    * Multiple `@` characters are allowed, so they are transferred
      into the patched object. This is only allowed for patching patches.
    * When applied, the `n` `@` chars from the patch will result
      in `n-1` `@` chars in the patched patch. That way,
      operator overrides can be propagated to arbitrarily nested patches.
  * The patch will fail to be loaded if:
    * The patch target is not known
    * Any of changed members is not known in the patch target
    * Any of the added parents is not known
    * -> Blind patching is not allowed
  * The patch will succeed to load if:
    * All members of the patch are available in the patch target
    * The patch target already inherits from a parent to be added
    * -> Inheritance patching doesn't conflict with other patches


#### Multi inheritance

The parents of a `nyan::Object` are kind of a mixin for members:

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
* The qualification is done by prefixing the precedes a `nyan::Object` name
  which is somewhere up the hierarchy and would grant conflict-free access
  to that member
* That does **not** mean the value somewhere up the tree is changed!
  The change is only defined in the current object, the qualification just
  ensures the correct target member is selected!


If one now has the `OHNoes` `nyan::Object` and desires to get values,
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

* Members of `nyan::Object` must have a type, which can be a
  * primitive type
    - `text`:     `"lol"`          - (duh.)
    - `int`:      `1337`           - (some number)
    - `float`:    `42.235`, `inf`  - (some floating point number)
    - `bool`:     `true`, `false`  - (some boolean value)
    - `file`:     `"./name" `      - (some filename, relative to the directory
                                      the defining nyan file is located at.
                                      If absolute, the path is relative to
                                      an engine defined root directory)
  * ordered set of elements of a type: `orderedset(type)`
  * set of elements of a type: `set(type)`
  * currently, there is **no** `list(type)` specified,
    but may be added later if needed
  * `nyan::Object`, to allow arbitrary hierarchies

* Type hierarchy
  * A `nyan::Object`'s type name equals its name: `A()` has type `A`
  * A `nyan::Object` `isinstance` of all the types of its parent `nyan::Object`s
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
    * union: ` += {..}`, `|= {..}` -> add values to set
    * subtract: `-= {..}` -> remove those values
    * intersection: `&= {..}` -> keep only values element of both
  * `orderedset(type)`:
    * assignment: `= o{value, value, ...}`
    * append: `+= o{..}` -> add values to the end
    * subtract: `-= o{..}`, `-= {..}` -> remove those values
    * intersection: `&= o{..}`, `&= {..}` -> keep only values element of both
  * TODO: `dict(keytype, valuetype)`:
    * assignment: `= {key: value, k: v, ...}`
    * insertion of data: `+= {k: v, ...}`, `|= {..}`
    * deletion of keys: `-= {k, k, ...}`, `-= {k: v, ..}`
    * keep only those keys: `&= {k, k, ..}`, `&= {k: v, ..}`
  * `nyan::Object` reference:
    * `= NewObject` set the reference to some other object.
      This reference must not be non-abstract (i.e. all members
      have a value defined). And it must be type-compatible, of course.



### Namespaces, imports and forward declarations

Namespaces and imports work pretty much the same way as Python defined it.
They allow to organize data in an easy hierarchical way on your file system.


#### Implicit namespace

A nyan file name is implies its namespace. That means the file name must
not contain a `.` (except the `.nyan`) to prevent naming conflicts.

`thuglife/units/backstreet.nyan`

Data defined in this file is in namespace:

`thuglife.units.backstreet`

An object is then accessed like:

`thuglife.units.backstreet.DrugDealer`


#### Importing

A file is loaded when another file imports it.
This is done by loading another namespace.

``` python
import thuglife
```

You can define convenience aliases of fully qualified names
with the `import ... (as ...)` statement.

This imports the namespace with an alias (right side)
which expands to the left side when used.

``` python

Frank(thuglife.units.backstreet.DrugDealer):
    speciality = "Meth"


# is the same as:

import thuglife.units.backstreet.DrugDealer as DrugDealer

Frank(DrugDealer):
    speciality = "Meth"


# which is also the same as

import thuglife.units.backstreet as thugs

Frank(thugs.DrugDealer):
    speciality = "Meth"
```


#### Cyclic dependencies

Inheritance can never be cyclic (duh). Member value usage can be cyclic.

Usage as member value can be done even though the object is not yet
declared. This works as objects in member values are always "pointers".

The compatibility for the value type is tested just when the
referenced object was actually loaded.
This means there are implicit forward declarations.

Example: deer death

``` python
# engine features:

Ability():
    ...

Behaviour():
    ...

Resource():
    name : text

DieAbility(Ability):
    die_animation : file
    become : Unit

Huntable(Ability):
    hunting_reaction : Behaviour

Unit():
    abilities : set(Ability)
    hp : int

ResourceAmount():
    type : Resource
    amount : float

ResourceSpot():
    resources : set(ResourceAmount)

IntelligentFlee(Behaviour):
    ...


# content pack:

Animal(Unit):
    ...

Deer(Animal):
    DeerDie(DieAbility):
        die_animation = "deer_die.png"
        become = DeadDeer

    DeerHuntable(Huntable):
        hunting_reaction = IntelligentFlee

    hp = 10
    graphic = "deer.png"
    abilities |= {DeerDie, DeerHuntable}

DeadDeer(Deer, ResourceSpot):
    DeerFood(ResourceAmount):
        type = Food
        amount = 250

    graphic = "dead_deer.png"
    resources = {DeerFood}
```


##### Forward declarations

The engine has to invoke the check whether all objects that were used as
forward declaration were actually defined.

If there are dangling forward declaration objects when invoking that
consistency check, a list of missing objects will be provided. These have
to be provided in order for nyan to load. Otherwise the objects affected
by incomplete members cannot be used.


##### Cyclic avoidance

If you encounter a cyclic dependency, try to redesign your data model by
extracting the common part as a separate object and then use it in both
old ones.


## nyan interpreter

`.nyan` files are read by the nyan interpreter part of `libnyan`.

* You feed `.nyan` files into the `nyan::Database`
* All data is loaded and checked for validity
* You can query any member and object of the store
* You can hold `nyan::Object`s as handles
* You can apply patches to any object at a given time, all already-applied patches after that time are undone
* All data history is stored over time


### Database views

Problem: Different players and teams have different states of the same nyan tree.

Solution: Hierarchy of state views.

A `nyan::View` has a parent which is either the root database or another `nyan::View`.

The view then stores the state for e.g. a player.

What does that mean?

* You can create a view of the main database
* You can create a view of a view
* Querying values respects the view the query is executed in
* If a patch is applied in a view, the data changes are applied in this view
  and all children of it. Parent view remain unaffected.

Querying data works like this:
* `nyan::Object obj = view.get(object_name)`
  * The `nyan::Object` is just a handle which is then used for real queries
* `obj.get(member_name, time)` will evaluates the member of the object at a give time
  * This returns the `nyan::Value` stored in the member at the given time.

Patching data works as follows:
* Obtain a patch object from some view
  * `nyan::Object patch = view.get(patch_name);`
  * If it is known in the view, return it
  * Else return it from the parent view
* Create a transaction with this Patch to change the view state at the desired time
  * `nyan::Transaction tx = view.new_transaction(time);`
* Add one or more patch objects to the transaction
  * `tx.add(patch); tx.add(...);`
  * `tx.add(another_patch, view.get(target_object_name))` is used to patch a child of
    the patch target.
* Commit the transaction
  * `bool success = tx.commit();`
  * This triggers, for each patch in the transaction:
    * Determine the patch target object name
      * If a custom patch target was requested,
        check if it was a child of the default patch target at loadtime.
    * Copy the patch target object in a (new) state at `time`
      * Query the view of the transaction at `time` for the target object, this may recursively query parent views
      * If there is no state at `time` in the view of the transaction, create a new state
      * Copy the target object into the state at `time` in the view of the transaction
    * Linearize the inheritance hierary to a list of patch objects
      * e.g. if we have a `SomePatch<TargetObj>()` and `AnotherPatch(SomePatch)` and we would like to apply `AnotherPatch`, this will result in `[SomePatch, AnotherPatch]`
    * Apply the list left to right and modify the copied target object
    * Notify child views that this patch was applied, perform the patch there as well

This approach allows different views of the database state and integrates with the
patch idea so e.g. team boni and player specific updates can be handled in an "easy"
way.



### Embedding nyan

A mod API could be implemented as follows: Create a `nyan::Object` named `Mod`
that has a member with a set of patches to apply. To add new data to the engine,
inherit from this `Mod`-object and add patches to the set. This `Mod`-object is
registered to the engine with a mod description file.

#### API definition example

In practice, this could look like this:

``` python
# Engine API definition: engine.nyan

Mod():
    patches : orderedset(Patch)

Tech():
    patches : orderedset(Patch)

Unit():
    hp : int
    can_create : set(Unit) = {}
    can_research : set(Tech) = {}

CFG():
    initial_buildings : set(Unit)
    name : text

StartConfigs():
    # available start game configurations
    available : set(CFG) = {}
```

``` python
# Data pack: pack.nyan

import engine

Villager(engine.Unit):
    hp = 100
    can_create = {TownCenter}

Loom(Tech):
    HPBoost<Villager>():
        hp += 50

    patches = {HPBoost}

TownCenter(engine.Unit):
    hp = 1500
    can_create = {Villager}
    can_research = {Loom}

DefaultConfig(engine.CFG):
    initial_buildings = {TownCenter}
    name = "you'll start with a town center"

DefaultMod(engine.Mod):
    Activate<engine.StartConfigs>():
        available += {DefaultConfig}

    patches = {Activate}
```

Mod information file `pack.nfo`:
``` ini
load: pack.nyan
mod: pack.DefaultMod
# could be extended with dependency and version information
```

#### Embedding in the engine

The mod API definitions in `engine.nyan` have to be designed exacly the way the
C++ engine code is then using it. It sets up the type system so that the nyan
C++ API can then be used to provide the correct information to the program that embeds nyan.

The load procedure and data access could be done like this:

1. Load `engine.nyan`
1. Read `pack.nfo`
1. Load `pack.nyan`
1. Apply "mod-activating" patches in `pack.DefaultMod`
1. Let user select one of `engine.StartConfigs.available`
1. Generate a map and place the `CFG.initial_buildings`
1. Display creatable units for each building on that map

When the newly created villager is selected, it can build towncenters!
And the towncenter can research a healthpoint-upgrade for villagers.

``` cpp
// callback function for reading nyan files via the engine
// we need this so nyan can access into e.g. archives of the engine.
std::string base_path = "/some/game/root";
auto file_fetcher = [base_path] (const std::string &filename) {
    return std::make_shared<File>(base_path + '/' + filename);
};

// initialization of API
auto db = std::make_shared<nyan::Database>();
db->load("engine.nyan", file_fetcher);

// load the userdata
ModInfo nfo = read_mod_file("pack.nfo");
db->load(nfo.load, file_fetcher);

// modification view: this is the changed database state
std::shared_ptr<nyan::View> root = db->new_view();

nyan::Object mod_obj = root->get(nfo.mod);
if (not mod_obj.extends("engine.Mod", 0)) { error(); }

nyan::OrderedSet mod_patches = mod_obj.get<nyan::OrderedSet>("patches", 0);

// activation of userdata (at t=0)
nyan::Transaction mod_activation = root->new_transaction(0);

for (auto &patch : mod_patches.items<nyan::Patch>()) {
    mod_activation.add(patch);
}

if (not mod_activation.commit()) { error("failed transaction"); }

// presentation of userdata (t=0)
for (auto &obj : root->get("engine.StartConfigs").get<nyan::Set>("available", 0).items<nyan::Object>()) {
    present_in_selection(obj);
}

// feedback from ui
nyan::Object selected_startconfig = ...;

// use result of ui-selection
printf("generate map with config %s", selected_startconfig.get<nyan::Text>("name", 0));
place_buildings(selected_startconfig.get<nyan::Set>("initial_buildings", 0));

// set up teams and players
auto player0 = std::make_shared<nyan::View>(root);
auto player1 = std::make_shared<nyan::View>(root);


// ====== let's assume the game runs now
run_game();


// to check if a unit is dead:
engine::Unit engine_unit = ...;
nyan::Object unit_type = engine_unit.get_type();
int max_hp = unit_type.get<nyan::Int>("hp", current_game_time);
float damage = engine_unit.current_damage();
if (damage > max_hp) {
    engine_unit.die();
}
else {
    engine_unit.update_hp_bar(max_hp - damage);
}

// to display what units a selected entity can build:
nyan::Object selected = get_selected_object_type();
if (selected.extends("engine.Unit", current_game_time)) {
    for (auto &unit : selected.get<nyan::Set>("can_create", current_game_time).items<nyan::Object>()) {
        display_creatable(unit);
    }
}

// technology research:
nyan::Object tech = get_tech_to_research();
std::shared_ptr<nyan::View> &target = target_player();
nyan::Transaction research = target.new_transaction(current_game_time);
for (auto &patch : tech.get<nyan::Orderedset>("patches", current_game_time).items<nyan::Patch>()) {
    research.add(patch);
}

if (not research.commit()) { error("failed transaction"); }
```


### Creating a scripting API

nyan does provide any possibility to execute code.
But nyan can be used as entry-point for full dynamic scripting APIs:
The names of hook functions to be called are set up through nyan.
The validity of code that is called that way is impossible to check,
so this can lead to runtime crashes.


## nyanc - the nyan compiler

**nyanc** can compile a .nyan file to a .h and .cpp file, this just creates
a new nyan type the same way the primitive types from above are defined.

Members can then be acessed directly from C++.

The only problem still unsolved with `nyanc` is:

If a "non-optimized" `nyan::Object` has multiple parents where some of them
were "optimized" and made into native code by `nyanc`, we can't select
which of the C++ objects to instanciate for it. And we can't create the
combined "optimized" object as the `nyan::Object` appeared at runtime.

This means we have to provide some kind of annotation, which of the parents
should be the annotated ones.

Nevertheless, `nyanc` is just an optimization, and has therefore no
priority until we need it.


## openage specific "standard library"

nyan in openage has specific requirements how to handle patches:
mods, technologies, technology-technologies.

### Defined `nyan::Object`s

The openage engine defines a few objects to inherit from.
The engine reacts differently when children of those `nyan::Object`s are
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
* `Movement`, `Gather`, `ResourceGenerator`,
  `ResourceSpot`, ... defined and implemented by engine as well
* The engine implements all kinds of things for the abilities
  and also triggers actions when the ability is invoked

##### `DropSite`: Object where resources can be brought to

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

`malte23 (instance) <- Crossbowman <- Archer <- RangedUnit (engine) <- Unit (engine) <- Object (built in)`

Why:

* There's a base nyan object, defined in the language internally
* The engine support units that move on screen
* The engine supports attack projectile ballistics
* All archers may receive armor/attack bonus updates
* Crossbowmen is an archer and can be built at the archery

`malte23` walks on your screen and dies laughing.
It is _not_ a `nyan::Object` but rather an unit object of the game engine
which references to the `Crossbowman` `nyan::Object` to get properties from.
`malte23` is handled in the unit movement system but the speed,
healthpoints and so on are fetched for malte's unit type, which is
`Crossbowman`, managed by nyan.


## Modding examples

### New resource

Let's create a new resource.

``` python

# Defined in the game engine:

Mod():
    name : text
    patches : set(Patch)

Building():
    name : text

Resource():
    name : text
    icon : file

DropSite():
    accepted_resources : set(Resource)


# Above are engine features.
# Lets create content in your official game data pack now:

Gold(Resource):
    name = "Bling bling"
    icon = "gold.svg"

Food(Resource):
    name = "Nom nom"
    icon = "food.svg"

TownCenter(Building, DropSite):
    name = "Town Center"
    accepted_resources = {Gold, Food}


# Now let's have a user mod that adds a new resource:

Silicon(Resource):
    name = "Silicon"

TCSilicon<TownCenter>():
    allowed_resources += {Silicon}

SiliconMod(Mod):
    name = "The modern age has started: Behold the microchips!"
    patches = {TCSilicon}

```

In the mod pack config file, `SiliconMod` is listed to be loaded.
That pack config format may be a simple .conf-style file.

When those nyan files are loaded, the all the objects are added. Your game
engine implements that the `SiliconMod` is displayed in some mod list and
that all `patches` from activated `Mod`s are applied at game start time.

The load order of the user supplied `Mod`s is to be determined by the
game engine. Either via some mod manager, or automatic resolution.
It's up to the engine to implement.


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
    updates : set(Patch)

Mod():
    name : text
    patches : set(Patch)

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


# User mod decreases the HP amount:
BalanceHP<LoomVillagerHP>():
    hp -= 5

LoomBalance(Mod):
    name = "Balance the Loom research to give"
    patches = {BalanceHP}

# in the mod pack metadata file, LoomBalance is denoted in the index.nfo
# to be loaded into the mod list of the engine.
```

### Creating a new ability

Now let's create the ability to teleport for the villager.

Abilities are used as [entity component system](https://en.wikipedia.org/wiki/Entity_component_system).
The game engine uses sets of those to modify unit behavior.


* Abilities can define properties like their animation
* An ability can be added as a tech to some units at runtime
  ("villagers and the tentacle monster can now teleport")
* Behavior must be implemented in the engine
  * If custom behavior is required, it must be set up through a scripting API of the engine
  * `nyan` can change and updated called function names etc to
    activate the scripting changes, but how is up to the engine


``` python

# The engine defines:
Mod():
    name : text
    patches : set(Patch)

Ability():
    mouse_animation : file

Unit():
    name : text
    hp : int
    abilities : set(Ability)

Resource():
    name : text
    icon : file

DropSite():
    accepted_resources : set(Resource)

Animation():
    image : file
    frames : int = 1
    loop : bool = true
    speed : float = 15.0

Ability():
    animation : Animation

CooldownAbility(Ability):
    recharge_time : float

Movement(Ability):
    speed : float
    instant : bool = false
    range : float = inf

CollectResource(Movement):
    target : Resource
    collect_animation : Animation


# Base game data defines:
Wood(Resource):
    name = "chop chop"
    icon = "wood.svg"

VillagerWalking(Animation):
    image = "walking_villager.png"
    frames = 18

VillagerMovement(Movement):
    animation = VillagerWalking
    speed = 15.0

WoodTransport(Animation):
    image = "wood_transport.png"
    frames = 20

WoodChop(Animation):
    image = "wood_transport.png"
    frames = 20

CollectWood(CollectResource):
    target = Wood
    animation = WoodTransport
    collect_animation = WoodChop
    speed = 12.0

Villager(Unit):
    name = "Villager"
    hp = 25
    abilities += {VillagerMovement, CollectWood}


# Teleport mod:
TeleportBlurb(Animation):
    image = "teleport_whooosh.png"
    frames = 10
    speed = 2

Teleport(Movement, CooldownAbility):
    speed = 0.0
    instant = true
    recharge_time = 30.0
    range = 5
    animation = TeleportBlurb

EnableTeleport<Villager>():
    abilities += {Teleport}

TeleportMod(Mod):
    name = "Awesome teleport feature to sneak into bastions easily"
    patches = {EnableTeleport}
```

* Why does `Teleport` inherit from both `Movement` and `CooldownAbility`?
  * Teleport is another movement variant, but the cooldown timer must be mixed in.
    After an ability was used, the engine checks if the `Ability` is a `CooldownAbility`,
    and then deactivates the ability for some time for that unit.
    When the engine checks `Teleport.extends(CooldownAbility)`,
    it is true and the timer routine will run.
* Why is there an `instant` member of `Movement`?
  * The game engine must support movement without pathfinding, otherwise even
    movement with infinite speed would be done by pathfinding.

This demonstrated that modding capabilities are strongly limited by the game
engine, nyan just assists you in designing a mod api in an intuitive way.
