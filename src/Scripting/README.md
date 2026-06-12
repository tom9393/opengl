# Scripting

Two scripting paths: native C++ behaviors and Lua scripts.

## Native

- **NativeScript** — base class with `OnCreate` / `OnUpdate` / `OnCollision` / `OnDestroyed` hooks, attached via `NativeScriptComponent`. `PlayerScript` is an example behavior.

## Lua

The Lua path uses **Sol2** over Lua 5.4 (exactly — 5.5 unsupported), attached via `LuaScriptComponent`.

- **ScriptEngine** — owns the Lua VM and the script lifecycle during play mode.
- **LuaBindings** — installs the C++ API (`Register`, split across the per-module registrars in [Bindings/](Bindings/)) and owns the per-entity lifecycle: `BindEntity` gives each scripted entity its own `sol::environment` seeded with `Self`/`Scene` and bound component handles (`Transform`, `RigidBody`, `Particles`, …), runs the script file into it, and calls `OnCreate`.
