# nyan Specification

## Table of Contents

* [1. Quick Reference](#quick-reference)
* [2. Object](#object)
* [3. Member](#member)
* [4. Patch](#patch)
* [5. Namespace](#namespace)
    * [5.1 Importing](#importing)
* [6. Examples](#examples)
    * [2.1 Patching a Patch](#patching-a-patch)
    * [2.2 Multi Inheritance](#multi-inheritance)

## Quick Reference

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
        another_member : type_name = value
        ...

    some_member : Inherited = NestedObject
    ...
```

## Object

An **object** declares a named storage space for key-value pairs.

```python
ObjName(ParentObj, OtherParentObj, ...):
    member_name : type_name
    other_member_name : type_name = value
    ...

    ParentObj.member_name = value
    ...

    NestedObject():
        another_member : type_name = value
        ...
```

The declaration of the object contains:

* A **name** for the object
* An arbritrary number of references to **[parent objects](#inheritance)**
* An arbritrary number of **[member](#member) definitions**
* Initialization of or changes to **inherited members**
* An arbritrary number of declarations of **nested objects**

The object name should use `CamelCase` notation.

All objects have a **type** whose name is equal to the object name. Additionally,
a nyan object inherit all types of their parents. For example, an object `B(A)`
has types `B` and `A`. Furthermore, all objects implicitly inherit from a
built-in object called `Object`, even if no parent is directly defined.

Members defined by the object must have a unique name. The member name should
use `snake_case` notation. In addition to the members defined by the object itself,
an object inherits all members of its parents. The value of inherited members
can be changed by the inheriting object, but their type cannot. References to
inherited members should be prefixed with the name of the ancestor object they are
inherited from. The prefix is optional if the inherited member's name does
not match any name of the unique members of the object.

Nested objects function like any other object. They allow for additional
structuring of the data model using the object hierarchy.

An object is **abstract** iff it contains at least one uninitialized member.
This includes inherited members.

If an object contains no member definitions, changes to inherited members
or nested objects, the object definition body must contain the `pass` keyword
instead.

```python
ObjName(ParentObj, OtherParentObj, ...):
    pass
```

Objects can be referenced by a fully qualifies object name (FQON). The FQON
is a unique string indentifier composed of the **namespace** the **object name**.
The namespace of an object is be derived from the location of the file it
is located in and its nesting parents (more details in section [Namespace](#namespace)).

```python
# file location: game/units/example.nyan

# FQON: game.units.example.ObjName
ObjName():
    ...

    # FQON: game.units.example.ObjName.NestedObject
    NestedObject():
        ...

    # FQON: game.units.example.ObjName.OtherNestedObject
    OtherNestedObject():
        ...

        # FQON: game.units.example.ObjName.OtherNestedObject.DeepNestedObject
        DeepNestedObject():
            ...
```


## Member

A **member** is a storage variable wth a predefined type.

A member is created by *declaring* it with a **name** and a **data type**.
The type cannot be changed once declared.

```python
SomeObject():
    member_name : type_name
```

A member is *initialized* by declaring it and assigning a **value** to it.
The value must correspond to the assigned type. For a full list of data
types and their possible values see the [member types doc](member_types.md).

```python
SomeObject():
    member_name : type_name = value
```

If the member was only declared when it was created, any child object can
still initialize it by assigning a value.

```python
SomeObject():
    member_name : type_name

ChildObject(SomeObject):
    SomeObject.member_name = value
```

A child object can also assign a new value to an inherited member that
was already initialized. This change is not backpropagated to the parent,
so other children are not affected.

```python
SomeObject():
    # Declaration and initialization: member_name = value
    member_name : type_name = value

ChildObject(SomeObject):
    # Assigns a new value: member_name = new_value
    SomeObject.member_name  = new_value

OtherChildObject(SomeObject):
    # Uses value of parent: member_name = value
    pass
```

A child object may use other type-specific operations than assignment
on an initialized inherited member. Most of the time, these are relative
operations.

The below example shows the *Addition* operation for the `int` data type.
For a full list of data types and their possible operators see the
[member types doc](member_types.md).

```python
SomeObject():
    # Declaration and initialization
    member_name : int = 10            # result: member_name = 10

ChildObject(SomeObject):
    # Adds 5 to the parent's member value
    SomeObject.member_name += 5       # result: member_name = 10 + 5 = 15

OtherChildObject(SomeObject):
    # Uses value of parent
    pass                              # result: member_name = 10
```

After a member has been initialized (or changed by inheritance), the
only way to alter a member value is through the application of a
**[patch](#patch)**.


## Patch

A **patch** is a special object type that can change member values
of another object and add new inheritance parents to it.

A patch always targets a specific object. The object must be defined.
Since patches are also objects, patches can target other patches
and themselves.

A patch is defined similar to a normal object with the addition of
a **target**. The patch target must be written in angled brackets
after the object name. The defined target cannot be changed.

```python
SomeObject():
    ...

PatchName<SomeObject>():
    ...
```

A patch **can** modify its target by:

* Changing a member value with an (type-specific) operator
* Replacing an operator of a member operation
* Adding additional parent objects

A patch **cannot**:

* Define its own members
* Add new members to the patch target
* Remove members from the patch target
* Redefine the data type of a member from the patch target
* Initialize members of the patch target
* Remove parents from the patch target

Member values are changed by using a type-specific operation on
the defined member. The below example shows the *Assignment* and
*Addition* operations for the `int` data type. For a full list of data
types and their possible operators see the [member types doc](member_types.md).

A patch can be applied multiple times. On every application all
operations are executed.

```python
SomeObject():
    member_name  : int = 7
    other_member : int = 23

SomePatch<SomeObject>():
    # Assigns 50 to member_name (on every application)
    member_name = 50             # result: member_name = 50
    # Adds 19 to other_member (on every application)
    other_member += 19           # result: other_member = 23 + 19 = 42 (1st application)
                                 # result: other_member = 42 + 19 = 61 (2nd application)
                                 # result: other_member = 61 + 19 = 80 (3rd application)
                                 # ...
```

Patches can target other patches in the same way.

```python
SomeObject():
    member_name  : int = 7
    other_member : int = 23

# Targets SomeObject
SomePatch<SomeObject>():
    member_name   = 50
    other_member += 19

# Targets SomePatch
OtherPatch<SomePatch>():
    # Adds 10 to the value that SomePatch assigns to member_name
    member_name  += 10           # resulting operation: member_name = 60
    # Subtracts 7 from the value that SomePatch add to other_member
    other_member -= 7            # resulting operation: other_member += 12
```

It should be stressed that by default, an operation only changes
the member *value* and not the *operator*. More specifically, the
operator of the changed member is not taken into account at all.

```python
SomeObject():
    member_name  : int = 7

SomePatch<SomeObject>():
    member_name -= 3

OtherPatch<SomePatch>():
    # Adds 10 to the VALUE that SomePatch assigns to member_name
    member_name += 10           # resulting operation: member_name -= 13

    # it DOES NOT result in: member_name += 7
```

However, it is possible to override the operator and the value
of a target's operation. To do this, the patch operator must be prefixed
with the `@` symbol.

```python
SomeObject():
    member_name  : int = 7

SomePatch<SomeObject>():
    member_name -= 3

OtherPatch<SomePatch>():
    # Replaces the whole operation on member_name in SomePatch
    member_name @+= 10           # resulting operation: member_name += 10
```

Overrides can be chained by adding multiple `@` override symbols.

```python
SomeObject():
    member_name  : int = 7

SomePatch<SomeObject>():
    member_name -= 3

OtherPatch<SomePatch>():
    member_name  = 1

FixOtherPatch<OtherPatch>():
    # Replaces the whole operation on member_name in SomePatch
    member_name @@+= 5           # resulting operation (OtherPatch): member_name @+= 5
                                 # resulting operation (SomePatch):  member_name  += 5
```

In the above example, the first `@` in `FixOtherPatch` marks an
override. All following `@`s are considered parts of the overriden
operation, which means they are copied along the operator and value
when the patch is applied. Therefore,the application of `FixOtherPatch`
will override the operation in `OtherPatch` with `member_name @+= 5`.
When `OtherPatch` is applied after that, it will now override the
operation in `SomePatch` with the operation defined after the `@`. The
result for `SomePatch` will be `member_name += 5`.

----

A patch adds additional parent objects to its target by specifying
a list of object references after the patch target. The patch must
specify for every parent whether it is appended to the front or the
end of the list of parents. This is done by prefixing (append end)
or suffixing (append front) the `+` symbol to the object reference.

```python
SomeObject():
    ...

PatchName<SomeObject>[+AdditionalParent, AnotherParent+, ...]():
    ...
```

Adding a parent must not induce name clashes of members (see the
[multiple inheritance example](#multi-inheritance)).


## Namespace

Namespaces allow the organization of data in a hierarchical way. A
namespace can be seen as an *address* inside a nyan data structure.

Every folder, `.nyan` file and nyan object implicitly defines its
namespace by its name and location inside the filesystem. File names
and folder names must not contain a `.` (except in the extension
`.nyan`) to prevent naming conflicts.

```
thuglife/units/backstreet.nyan
```

Data defined in this file is in namespace:

```
thuglife.units.backstreet
```

An object in the file is then accessed like this via its FQON:

```
thuglife.units.backstreet.DrugDealer
```

Overall, the example defines (at least) 4 namespaces:

```
thuglife                                # folder namespace
thuglife.units                          # folder namespace
thuglife.units.backstreet               # file namespace
thuglife.units.backstreet.DrugDealer    # object namespace
```


### Importing

To reference nyan objects in other files, a namespace they are
in has to be imported using the `import` keyword.

``` python
import thuglife

Frank(thuglife.units.backstreet.DrugDealer):
    speciality = "Meth"
```

Aliases of namespaces can be defined for convenience with the
`import ... (as ...)` statement. An alias must be unique for
every file.


``` python
import thuglife.units.backstreet.DrugDealer as Dealer

Frank(Dealer):
    speciality = "Meth"
```

Any intermediate alias also works. The object reference must then
be defined as relative to the namespace defined by the alias.

``` python
import thuglife.units.backstreet as thugs

Frank(thugs.DrugDealer):
    speciality = "Meth"
```

## Examples

### Patching a Patch

A user mod that patches loom to increase villager hp by 10 instead of 15.

0. Loom is defined in the base data pack
1. The mod defines to update the original loom tech
2. The tech is researched, which applies the updated loom tech to the
   villager instance of the current player

``` python
# Base game data defines:
Villager(Unit):
    name = "Villager"
    hp = 25

LoomVillagerHP<Villager>():
    hp += 15

# User mod decreases the HP amount:
BalanceHP<LoomVillagerHP>():
    hp -= 5
```

This demonstrated that modding capabilities are strongly limited by the game
engine, nyan just assists you in designing a mod api in an intuitive way.


### Multi inheritance

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
