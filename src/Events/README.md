# Events

Typed publish-subscribe event system.

- **Event** — base class; every event has an `EventType` (e.g. `KeyboardPress`, `MouseMoved`, `WindowResize`) and category flags (Keyboard, Mouse, Application, Window) used for filtering.
- **EventBroker** — the dispatcher. `Subscribe<T>(callback, owner)` registers a type-safe handler (the broker downcasts for you), `SubscribeCategory` registers for a whole category, and `UnsubscribeAll(owner)` removes every subscription tied to an owner pointer — subscribers pass `this` so teardown is one call.
- **ApplicationEvent / KeyboardEvent / MouseEvent** — the concrete event types: window resize/close, key press/release/type, mouse move/scroll/button.

Events originate in the platform window's GLFW callbacks and flow through `Application` into the layer stack and broker subscribers.
