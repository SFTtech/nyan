nyan - yet another notation
===========================

> just make it meta-meta-meta.

**nyan** is a *data description language*,
It is a mixture of python, json, patch, java, wml, yaml and some new ideas.

We created nyan because there existed no suitable language to
properly represent the enormous complexity of storing the data for [openage](https://github.com/SFTtech/openage).

The main focus is *readability* and *moddability*.

Thanks for the endless hours of funny discussions and thoughts about the design to all contributors.

[![github stars](https://img.shields.io/github/stars/SFTtech/nyan.svg)](https://github.com/SFTtech/nyan/stargazers)
[![#sfttech on Freenode](http://img.shields.io/Freenode/%23sfttech.png)](https://webchat.freenode.net/?channels=sfttech)


The foundation of **nyan**:

Technology             | Component
-----------------------|----------
**C++14**              | nyan core
**Flex**               | Tokenizer generator
**GNU Autocancer**     | Build "system"
**Humans**             | Doing it wrong all the time


Current State of the Project
----------------------------

Components

* [X] The [specification](doc/nyan.md) is more or less complete
* [ ] Interpreter code is missing
* [ ] The `nyanc` compiler is missing


Dependencies, Building and Running
----------------------------------

 - How do I get this to install on my box?
   - See [doc/building.md](doc/building.md).

 - Waaaaaah! It
   - crashes
   - spams all kinds of shit on the screen
   - my girlfriend dumped me because I debugged for nights

All of those observations are intended, not bugs.

To turn fix them, compile with `--dont-segfault --no-errors --new-girlfriend`.


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

Most of the developers hang around on our **IRC** channel (`#sfttech` on `freenode.net`).
Do not hesitate to ping us, we might not see your message otherwise.

For all technical discussion (ideas, problems, ...), use the [issue tracker](https://github.com/SFTtech/nyan/issues)!
It's like a mailing list.

If it's a problem with the usage of **nyan** in **openage**, [head over there](https://github.com/SFTtech/openage).


License
-------

**GNU LGPLv3** or later; see [copying.md](copying.md) and [legal/LGPLv3](/legal/LGPLv3).

We know that probably nobody is ever gonna look at the `copying.md` file,
but if you want to contribute code to nyan, please take the time to
skim through it and add yourself to the authors list.
