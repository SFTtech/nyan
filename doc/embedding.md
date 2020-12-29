# Embedding nyan into a Game Engine

## nyan interpreter

`.nyan` files are read by the nyan interpreter part of `libnyan`.

* You feed `.nyan` files into the `nyan::Database`
* All data is loaded and checked for validity
* You can query any member and object of the store
* You can hold `nyan::Object`s as handles
* You can apply patches to any object at a given time, all already-applied patches after that time are undone
* All data history is stored over time


## Embedding in the Engine Code

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


#### API definition example

openage uses an [ECS-style nyan API](https://github.com/SFTtech/openage/tree/master/doc/nyan/api_reference) for storing game data.


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
