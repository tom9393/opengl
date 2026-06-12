# Physics

Bullet3 integration.

- **PhysicsEngine** — wraps the Bullet world: creation, stepping during play mode, collision dispatch to scripts, and runtime start/stop bracketing.
- **RigidBody** — RAII wrapper around a `btRigidBody` + motion state, carried at runtime by a `RigidBodyComponent`. Built from an `RBShape` (`RBBoxShape` / `RBConvexShape` / `RBComplexShape` / `RBPlaneShape`) and exposing the gameplay API: impulses, velocities, kinematic toggles, etc. Complex shapes use convex hulls baked by `Asset/CollisionBaker` (`.pmcol` files).
- **PhysicsDebugDrawer** — Bullet debug-draw implementation that visualizes collision shapes as lines in the viewport.
