-- Pushed.lua

function ApplyForce(direction, intensity)
    RigidBody:ApplyImpulse(direction, intensity)
end

function PushPlayerBack(other)
    local environment = other:GetLuaEnvironment()
    local target = environment.Transform.Translation
    RigidBody:LookAt(target)
    print("Done")
end