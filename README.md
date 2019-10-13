nyan - yet another notation
===========================

> just make it meta-meta-meta.

**nyan** is a *data description language*,
It is a mixture of python, json, patch, wml, yaml and some new ideas.

It stores hierarchical objects with key-value pairs in a database with the key idea that changes in a parent affect all children.

We created nyan because there existed no suitable language to
properly represent the enormous complexity of storing the data for [openage](https://github.com/SFTtech/openage).

The main focus is *readability* and *moddability*.

[![github stars](https://img.shields.io/github/stars/SFTtech/nyan.svg)](https://github.com/SFTtech/nyan/stargazers)
[![#sfttech on Freenode](https://img.shields.io/badge/chat-on%20freenode-brightgreen)](https://webchat.freenode.net/?channels=sfttech)


The foundation of **nyan**:

Technology             | Component
-----------------------|----------
**C++17**              | nyan core
**Flex**               | Tokenizer generator
**CMake**              | Build "system"
**Humans**             | Doing it wrong all the time


How?
----

Let's assume we have a fun λ half-life strategy game.

The game engine provides some functionality which is exposed by `nyan`.

``` python
# This is the content and mod API of the Engine:

Unit():
    hp : int
    animation : file

Building():
    hp : int
    creates : set(Unit)
    model : file
```

Using this, the base game pack provides actual game content:

``` python
# base_game_data.nyan

OverwatchSoldier(Unit):
    hp = 50
    animation = "./assets/soldier.ani"

Strider(Unit):
    hp = 2100
    animation = "./assets/strider.ani"

CombineCitadel(Building):
    hp = 9001
    creates = {OverwatchSoldier, Strider}
    model = "./assets/lambda_hq.mdl"

Citizen(Unit):
    hp = 60
    animation = "./assets/male09.ani"

# gordon is a citizen with more hp
Gordon(Citizen):
    hp += 40
    animation = "./assets/gordon.ani"

RebelHQ(Building):
    hp = 5000
    creates = {Citizen, Gordon}
    model = "./assets/lambda_hq.mdl"
```

Now, let's create a mod that adds the overwatch elite
and gives the striders even more hp.

``` python
# elite_strider_mod.nyan

# create a new unit:
OverwatchElite(Unit):
    hp = 70
    animation = "./assets/coolersoldier.ani"

# change the strider:
ChangeStrider<Strider>():
    hp += 1000

# change the citadel to build the elite:
AddElite<CombineCitadel>():
    creates += {OverwatchElite}

# create a mod that informs the engine about its patches
StriderEliteMod(Mod):
    name = "Add the elite and make striders stronger"
    patches = {AddElite, ChangeStrider}
```

When the engine activates the mod ("applies the patches"),
the combine citadel can create the new unit and the strider is stronger.


The fun begins if you now create a mod that mods the mod.
Which is totally possible with **nyan**.


Specification
-------------

Read the [specification](doc/nyan.md).


Current State of the Project
----------------------------

`nyan` is fully functional and can be used in your project.

There's some features left to implement, but those only
cover special use cases:

* [ ] Inverse patch generation
* [ ] Subobject set specializations
* [ ] Callback trigger when a value changes
* [ ] Member name qualifications for name conflict resolving


Dependencies, Building and Running
----------------------------------

 - How do I get this to install on my box?
   - See [doc/building.md](doc/building.md).

 - Waaaaaah! It
   - crashes
   - spams all kinds of shit on the screen
   - my girlfriend dumped me because I debugged for nights

All of those observations are intended, not bugs.

To get rid of them, recompile with `--dont-segfault --shut-up --new-girlfriend`.

Operating System    | Build status
--------------------|--------------
Debian Sid          | [Todo: Kevin #11](https://github.com/SFTtech/kevin/issues/11)
MacOSX 10.14        | [![Build Status](https://travis-ci.org/SFTtech/nyan.svg?branch=master)](https://travis-ci.org/SFTtech/nyan)
Windows 10 - x64    | [![Build status](https://ci.appveyor.com/api/projects/status/6t1yonx5fu7dejs0/branch/master?svg=true)](https://ci.appveyor.com/project/simonsan/nyan-c53id/branch/master)

If this still does not help, try the [contact section](#contact)
or the [bug tracker](https://github.com/SFTtech/nyan/issues).


Development Process
-------------------

How does contributing work here?
 - Extensive [syncronization](#contact)!
 - Report [issues](https://github.com/SFTtech/nyan) and send [pull requests](https://help.github.com/articles/using-pull-requests/).


The documentation is also in this repo:

- Code documentation is embedded in the sources for Doxygen (see [doc readme](doc/README.md)).
- Have a look at the [doc directory](doc/). This folder tends to outdate when code changes.


Contact
-------

If you have the desire to perform semi-human interaction,
join our **Matrix** or **IRC** chatroom!

* [`#sfttech:matrix.org`](https://riot.im/app/#/room/#sfttech:matrix.org)
* [`irc.freenode.net #sfttech`](https://webchat.freenode.net/?channels=sfttech)

For ideas, problems, ..., use the [issue tracker](https://github.com/SFTtech/nyan/issues)!

If it's a problem with the usage of **nyan** in **openage**, [head over there](https://github.com/SFTtech/openage).


License
-------

**GNU LGPLv3** or later; see [copying.md](copying.md) and [legal/LGPLv3](/legal/LGPLv3).

We know that probably nobody is ever gonna look at the `copying.md` file,
but if you want to contribute code to nyan, please take the time to
skim through it and add yourself to the authors list.
