# nyan Member Types

This document contains an overview of the available nyan member types and
operations that are allowed on them.

## Table of Contents

* [1. Quick Reference](#quick-reference)
    * [1.1 Primitive Types](#primitive-types)
    * [1.2 Complex Types](#complex-types)
    * [1.3 Type Modifiers](#type-modifiers)
* [2. Data Types](#data-types)
    * [2.1 int](#int)
    * [2.2 float](#float)
    * [2.3 bool](#bool)
    * [2.4 text](#text)
    * [2.5 file](#file)
    * [2.6 set](#set)
    * [2.7 orderedset](#orderedset)
    * [2.8 dict](#dict)
    * [2.9 object](#object)
* [3. Type Modifiers](#type-modifiers-1)
    * [3.1 optional](#optional)
    * [3.2 children](#children)
* [4. Rules for Operations with Infinity](#rules-for-operations-with-infinity)

## Quick Reference

### Primitive Types

nyan Type | C++ Equivalent | Operators               | Description
----------|----------------|-------------------------|-----------------------------------
`int`     | `int32_t`      | `=`,`+=`,`-=`,`*=`,`/=` | 32-Bit signed integer.
`float`   | `float`        | `=`,`+=`,`-=`,`*=`,`/=` | 32-Bit floating point value.
`bool`    | `bool`         | `=`,`&=`,`\|=`           | Boolean value.
`text`    | `std:string`   | `=`,`+=`                | String of characters.
`file`    | `std:string`   | `=`                     | Path to a file.


### Complex Types

nyan Type     | C++ Equivalent       | Operators                | Description
--------------|----------------------|--------------------------|-----------------------------------
`set`         | `std::set`           | `=`,`+=`,`-=`,`&=`,`\|=` | Set of primitive values or nyan objects.
`ordered_set` | `std::ordered_set`   | `=`,`+=`,`-=`,`&=`,`\|=` | Ordered set of primitive values or nyan objects.
`dict`        | `std::unordered_map` | `=`,`+=`,`-=`,`&=`,`\|=` | Key-value pairs of primitive values or nyan objects.
`object`      | -                    | `=`                      | Reference to a nyan object.


### Type Modifiers

Type Modifier | nyan Types | Description
--------------|------------|-----------------------
`optional`    | All        | The member can be initialized with the placeholder value `None`.
`children`    | `object`   | Only descendants of the object can be assigned, not the object itself.


## Data Types

### `int`

A member with type `int` can store a signed 32-Bit integer value. Values must be
in the range of `-2147483648` to `2147483647`. Additionally, positive infinity
(`inf`) and negative infinity (`-inf`) are supported.

```python
SomeObject():
    a : int         # only declaration
    b : int = 5     # declaration and initialization with 5
    c : int = -20   # declaration and initialization with -20
```


#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator | Operation      | Examples                | Description
---------|----------------|-------------------------|------------------------------------
`=`      | Assignment     | `a = 5`                 | Assigns the operand value to the member.
`+=`     | Addition       | `a += 5`                | Adds the operand value to the current member value.
`-=`     | Subtraction    | `a -= 5`                | Subtracts the operand value from the current member value.
`*=`     | Multiplication | `a *= 2`  `a *= 2.5f` | Multiplies the operand value with the current member value. Floats can be used as an operand. The result is floored.
`/=`     | Division       | `a /= 2`  `a /= 2.5f` | Divides the current member value by the operand value. Floats can be used as an operand. The result is floored.

#### Usage Examples

```python
SomeObject():
    a : int = 10
    b : int = -5
    c : int = 20
    d : int = 1

Patch1<SomeObject>():
    a = 5    # result: a = 5  (reassignment)
    b += 7   # result: b = 2  (addition: -5 + 7)
    c -= 5   # result: c = 15 (sutraction: 20 - 5)
    d *= 5   # result: d = 5  (multiplication: 1 * 5)

OtherObject():
    a : int = -7
    b : int = 3
    c : int = 4
    d : int = 8

Patch2<OtherObject>():
    a -= 5    # result: a = -12 (subtraction: -7 - 5)
    b -= -2   # result: b = 5   (addition: 3 - (-2))
    c *= 2.5f # result: c = 10  (multiplication: 4 * 2.5)
    d /= 3    # result: d = 2   (division: 8 / 3 = 2.666... floored to 2)
```


### `float`

A member with type `float` can store a single precision floating point value.
Positive infinity (`inf`) and negative infinity (`-inf`) are supported.

```python
SomeObject():
    a : float            # only declaration
    b : float = 5.0f     # declaration and initialization with 5.0
    c : float = 8.3456f  # declaration and initialization with 8.3456
    d : float = -20.2f   # declaration and initialization with -20.2
```


#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator | Operation      | Examples    | Description
---------|----------------|-------------|------------------------------------
`=`      | Assignment     | `a = 5.0f`  | Assigns the operand value to the member.
`+=`     | Addition       | `a += 5.0f` | Adds the operand value to the current member value.
`-=`     | Subtraction    | `a -= 5.0f` | Subtracts the operand value from the current member value.
`*=`     | Multiplication | `a *= 2.5f` | Multiplies the operand value with the current member value.
`/=`     | Division       | `a /= 2.5f` | Divides the current member value by the operand value.


#### Usage Examples


```python
SomeObject():
    a : float = 10.2f
    b : float = -5.1f
    c : float = 20.6f
    d : float = 0.4f

Patch1<SomeObject>():
    a = 5.5f    # result: a = 5.5  (reassignment)
    b += 1.2f   # result: b = 3.9  (addition: -5.1 + 1.2)
    c -= 5.0f   # result: c = 15.6 (sutraction: 20.6 - 5.0f)
    d *= 2.0f   # result: d = 0.8  (multiplication: 0.4 * 2.0)

OtherObject():
    a : float = -7.5f
    b : float = 3.3f
    c : float = 4.0f
    d : float = 4.9f

Patch2<OtherObject>():
    a -= 5.0f    # result: a = -12.5 (subtraction: -7.5 - 5.0)
    b -= -2.0f   # result: b = 5.3   (addition: 3.3 - (-2.0))
    c *= 0.5f    # result: c = 2.0   (multiplication: 4.0 * 0.5)
    d /= 7.0f    # result: d = 0.7   (division: 4.9 / 7.0)
```


### `bool`

A member with type `bool` can store a boolean value (`True` or `False`).

```python
SomeObject():
    a : bool          # only declaration
    b : bool = True   # declaration and initialization with True
    c : bool = False  # declaration and initialization with False
```

#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator | Operation      | Examples      | Description
---------|----------------|---------------|------------------------------------
`=`      | Assignment     | `a = True`    | Assigns the operand value to the member.
`&=`     | Logical AND    | `a &= False`  | Result is `True` iff both the operand value **and** the current member value are `True`, else `False`.
`\|=`     | Logical OR    | `a \|= False` | Result is `True` if either the operand value **or** the current member value are `True`, else `False`.


#### Usage Examples


```python
SomeObject():
    a : bool = True
    b : bool = True
    c : bool = True
    d : bool = False

Patch<SomeObject>():
    a = False   # result: a = False (reassignment)
    b &= True   # result: b = True  (True AND True)
    c &= False  # result: c = False (True AND False)
    d |= True   # result: d = True  (False OR True)
```


### `text`

A member with type `text` can store an UTF-8 encoded string.

```python
SomeObject():
    a : text                        # only declaration
    b : text = "This is a string!"  # declaration and initialization
```

#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator | Operation      | Examples     | Description
---------|----------------|--------------|------------------------------------
`=`      | Assignment     | `a = "bla"`  | Assigns the operand value to the member.
`+=`     | Append         | `a += "abc"` | Append operand string to end of the current member value.


#### Usage Examples


```python
SomeObject():
    a : text = "abc"
    b : text = "abrakadabra "

Patch<SomeObject>():
    a = "xyz"           # result: a = "xyz" (reassignment)
    b += "simsalabim"   # result: b = "abrakadabra simsalabim"
```


### `file`

A member with type `text` can store an absolute or relative path to a file
as an UTF-8 encoded string. `/` should be used as a file separator. For
relative path traversal, `.` (current folder) and `..` (parent folder)
can be used.

```python
SomeObject():
    a : file                          # only declaration
    b : file = "/abs/path/to/file"    # declaration and initialization of absolute path
    c : file = "../rel/path/to/file"  # declaration and initialization of relative path
```

#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator | Operation      | Examples     | Description
---------|----------------|--------------|------------------------------------
`=`      | Assignment     | `a = "bla"`  | Assigns the operand value to the member.


#### Usage Examples


```python
SomeObject():
    a : file = "path/to/file"

Patch<SomeObject>():
    a = "new/path/to/file"  # result: a = "new/path/to/file" (reassignment)
```


### `set`

A member with type `set` can store a collection of items with a predefined
type. The allowed item type must be specified during the member declaration.
Sets cannot contain duplicates of an item. They can be empty.

`set` does not preserve the input order of items.


```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

DifferentChildObject(OtherObject):
    pass

SomeObject():
    a : set(OtherObject)                    # only declaration
    a : set(OtherObject) = {}               # declaration and initialization with empty set
    b : set(OtherObject) = {ChildObject}    # declaration and initialization with {ChildObject}
    c : set(OtherObject) = {OtherObject,    # declaration and initialization with multiple items
                            ChildObject,
                            DifferentChildObject}
```


#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator    | Operation      | Examples                              | Description
------------|----------------|---------------------------------------|------------------------------------
`=`         | Assignment     | `a = {<items>}`                       | Assigns the operand value to the member.
`+=`, `\|=` | Union          | `a += {<items>}`<br>`a \|= {<items>}` | Performs a union of the operand and the current member value. Items from the operand are added to the member set.
`-=`        | Complement     | `a -= {<items>}`                      | Performs a complement of the operand and the current member value. Items from the operand are removed from the member set.
`&=`        | Intersection   | `a &= {<items>}`                      | Performs an intersection of the operand and the current member value. Items that are not in both the operand and the member set are removed from the member set.


#### Usage Examples


```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

DifferentChildObject(OtherObject):
    pass

SomeObject():
    a : set(OtherObject) = {}
    b : set(OtherObject) = {OtherObject}
    c : set(OtherObject) = {ChildObject}
    d : set(OtherObject) = {ChildObject, DifferentChildObject}

Patch<SomeObject>():
    a = {DifferentChildObject}       # result: a = {DifferentChildObject}              (reassignment)
    b += {DifferentChildObject}      # result: b = {OtherObject, DifferentChildObject} (union)
    c -= {ChildObject}               # result: c = {}                                  (complement)
    d &= {ChildObject, OtherObject}  # result: d = {ChildObject}                       (intersection)
```


### `orderedset`

A member with type `orderedset` can store a collection of items with a predefined
type. The allowed item type must be specified during the member declaration.
Sets cannot contain duplicates of an item. They can be empty.

`orderedset` does preserve the input order of items.


```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

DifferentChildObject(OtherObject):
    pass

SomeObject():
    a : orderedset(OtherObject)                     # only declaration
    a : orderedset(OtherObject) = o{}               # declaration and initialization with empty set
    b : orderedset(OtherObject) = o{ChildObject}    # declaration and initialization with o{ChildObject}
    c : orderedset(OtherObject) = o{OtherObject,    # declaration and initialization with multiple items
                                    ChildObject,
                                    DifferentChildObject}
```


#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator    | Operation      | Examples                                | Description
------------|----------------|-----------------------------------------|------------------------------------
`=`         | Assignment     | `a = o{<items>}`                        | Assigns the operand value to the member.
`+=`, `\|=` | Union          | `a += o{<items>}`<br>`a \|= o{<items>}` | Performs a union of the operand value and the current member value. Items from the operand are added to the member set.
`-=`        | Complement     | `a -= o{<items>}`; `a -= {<items>}`     | Performs a complement of the operand value and the current member value. Items from the operand are removed from the member set.
`&=`        | Intersection   | `a &= o{<items>}`; `a &= {<items>}`     | Performs an intersection of the operand value and the current member value. Items that are not in both the operand and the member set are removed from the member set.


#### Usage Examples


```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

DifferentChildObject(OtherObject):
    pass

SomeObject():
    a : orderedsetset(OtherObject) = o{}
    b : orderedsetset(OtherObject) = o{OtherObject}
    c : orderedsetset(OtherObject) = o{ChildObject}
    d : orderedsetset(OtherObject) = o{ChildObject, DifferentChildObject}

Patch<SomeObject>():
    a = o{DifferentChildObject}       # result: a = o{DifferentChildObject}              (reassignment)
    b += o{DifferentChildObject}      # result: b = o{OtherObject, DifferentChildObject} (union)
    c -= o{ChildObject}               # result: c = o{}                                  (complement)
    d &= o{ChildObject, OtherObject}  # result: d = o{ChildObject}                       (intersection)
```


### `dict`

A member with type `dict` can store a collection of key-value pairs.
Both the key type and value type must be specified during the member declaration.
Dicts cannot contain items with duplicate keys. They can be empty.

```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

DifferentChildObject(OtherObject):
    pass

SomeObject():
    a : dict(OtherObject, int)                       # only declaration
    b : dict(OtherObject, int) = {}                  # declaration and initialization with empty dict
    c : dict(OtherObject, int) = {ChildObject: 2}    # declaration and initialization with single item
    d : dict(OtherObject, int) = {OtherObject: 3,    # declaration and initialization with multiple items
                                  ChildObject: -15,
                                  DifferentChildObject: inf}
```

#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator    | Operation        | Examples                               | Description
------------|------------------|----------------------------------------|------------------------------------
`=`         | Assignment       | `a = {key: val}`                       | Assigns the operand value to the member.
`=`         | Keyed Assignment | `a[key] = val`                         | Assigns the operand value to the specified key. If the key already exists, its previous value is overwritten.
`+=`, `\|=` | Union            | `a += {key: val}`<br>`a \|= {<items>}` | Performs a union of the operand value and the current member value. Items from the operand are added to the member dict.
`-=`        | Complement       | `a -= {key}`                           | Performs a complement of the operand value and the current member value. Items using a key from the operand set are removed from the member dict.
`&=`        | Intersection     | `a &= {key}`                           | Performs an intersection of the operand value and the current member value. Items using a key that is not in both the operand set and the member dict are removed from the member dict.


#### Usage Examples


```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

DifferentChildObject(OtherObject):
    pass

SomeObject():
    a : dict(OtherObject, int) = {}
    b : dict(OtherObject, int) = {}
    c : dict(OtherObject, int) = {}
    d : dict(OtherObject, int) = {ChildObject: 2}
    e : dict(OtherObject, int) = {ChildObject: 5,
                                  DifferentChildObject: -10}

Patch<SomeObject>():
    a = {OtherObject: 50}            # result: a = {OtherObject: 50}    (reassignment)
    b[OtherObject] = -1000           # result: b = {OtherObject: -1000} (keyed assignment)
    c += {ChildObject: 5}            # result: c = {ChildObject: 5}     (union)
    d -= {ChildObject}               # result: d = {}                   (complement)
    e &= {ChildObject, OtherObject}  # result: e = {ChildObject: 5}     (intersection)
```


### `object`

A member with type `object` can store a nyan object reference.

```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

SomeObject():
    a : OtherObject                # only declaration
    b : OtherObject = OtherObject  # declaration and initialization with OtherObject
    c : OtherObject = ChildObject  # declaration and initialization with ChildObject
```


#### Operators

The following operators can be used when inheriting from or patching a nyan object.

Operator   | Operation      | Examples        | Description
-----------|----------------|-----------------|------------------------------------
`=`        | Assignment     | `a = Object`    | Assigns the operand value to the member.


#### Usage Examples


```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

SomeObject():
    a : OtherObject = OtherObject

Patch<SomeObject>():
    a = ChildObject       # result: a = ChildObject (reassignment)
```


## Type Modifiers

### `optional`

A member with type modifier `optional` can have the placeholder value
`None` assigned. `None` indicates that the member is initialized, but
has no regular value. `None` can only be assigned and not used as a
second operand in other operations. Additionally, operations other than
assignment have no effect on members that have `None` assigned.

Members of any data type can have the `optional` modifier. `optional`
members still need to be initialized with either `None` or a regular
value to not be abstract.

```python
SomeObject():
    a : optional(int)                 # declaration of an int as optional
    b : optional(float)               # declaration of a float as optional
    c : optional(set(OtherObject))    # declaration of a set(OtherObject) as optional
    d : optional(OtherObject)         # declaration of a nyan object reference to OtherObject as optional

OtherObject():
    a : optional(int) = None                 # declaration and initialization with None
    b : optional(OtherObject) = OtherObject  # declaration and initialization with regular value
```

#### Usage Examples


```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

SomeObject():
    a : optional(int) = 5
    b : optional(float) = None
    c : optional(set(OtherObject)) = None
    d : optional(OtherObject) = OtherObject

Patch<SomeObject>():
    a = None         # result: a = None        (reassignment)
    b += 10.0f       # result: b = None        (no effect)
    c = {}           # result: c = {}          (reassignment)
    d = ChildObject  # result: d = ChildObject (reassignment)
```


### `children`

`children` is a type modifier for the `object` data type. It signifies
that only references to the descandents of a nyan object can be assigned
as member values. References to the object with the same identity are not
allowed.

```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

SomeObject():
    a : children(OtherObject)                # declaration of a to allow only descendants of OtherObject
    b : children(OtherObject) = ChildObject  # declaration of b to allow only descendants of OtherObject
                                             # and initialization with ChildObject
    c : children(OtherObject) = OtherObject  # NOT ALLOWED
```

#### Usage Examples


```python
OtherObject():
    pass

ChildObject(OtherObject):
    pass

DifferentChildObject(OtherObject):
    pass

SomeObject():
    a : children(OtherObject) = DifferentChildObject
    b : children(OtherObject) = ChildObject
    c : children(OtherObject) = ChildObject

Patch<SomeObject>():
    a = ChildObject            # result: a = ChildObject          (reassignment)
    b = DifferentChildObject   # result: b = DifferentChildObject (no effect)
    c = OtherObject            # NOT ALLOWED
```



## Rules for Operations with Infinity

Members of type `int` or `float` support the assignment of positive infinity (`inf`)
and negative infinity (`-inf`). Infinity should not be seen as a regular value.
Instead, `inf` is *interpreted* as a value that is higher (lower for `-inf`)
than the maximum possible integer/float value. nyan will handle operations involving
`inf` that are not assignments according to the standard rules of calculus (see
Examples section).

Operations that would lead to nyan calculating

* `inf - inf`
* `inf / inf`
* `inf * 0`

will throw an error, since the result is undefined. This situation can generally
be avoided by not using `inf` as a second operand for any operation other than
assignment.


### Examples

Using infinity as the first operand and a regular value as the second operand:

```python
SomeObject():
    a : int = inf
    b : int = inf
    c : int = inf
    d : int = inf
    e : int = inf

Patch1<SomeObject>():
    a += 5   # result: a = inf
    b -= 5   # result: b = inf
    c *= 5   # result: c = inf
    d *= -5  # result: d = -inf
    e *= 0   # NOT ALLOWED
    f /= 5   # result: f = inf
```

Using a regular value as the first operand and infinity as the second operand:

```python
SomeObject():
    a : int = 5
    b : int = 5
    c : int = 5
    d : int = 5
    e : int = 5

Patch2<SomeObject>():
    a += inf   # result: a = inf
    b -= inf   # result: b = -inf
    c *= inf   # result: c = inf
    d *= -inf  # result: d = -inf
    e /= inf   # result: e = 0
```

Using infinity as the first and second operand:

```python
SomeObject():
    a : int = inf
    b : int = inf
    c : int = inf
    d : int = inf
    e : int = inf
    f : int = inf

Patch3<SomeObject>():
    a += inf   # result: a = inf
    b *= inf   # result: b = inf
    c *= -inf  # result: c = -inf
    d -= inf   # NOT ALLOWED
    e += -inf  # NOT ALLOWED
    f /= inf   # NOT ALLOWED
```
