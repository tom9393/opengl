-- Push.lua

function OnCreate()
    print("Push script created!")
end

-- Configuration
local radius = 10.0
local speed = 2.0
local center = { x = 0, y = 2, z = 0 } -- Height of 5 units

-- Internal state
local timer = 0.0

function OnUpdate(ts)
    timer = timer + (ts * speed)

    -- 1. Calculate the target position on the circle (XZ Plane)
    local targetX = center.x + math.cos(timer) * radius
    local targetZ = center.z + math.sin(timer) * radius
    local targetPos = vec3.new(targetX, center.y, targetZ)

    -- 2. Get the Physics component
    local rb = RigidBody
    
    if rb then
        -- OPTION A: For Kinematic Bodies (Precise follow)
        -- This moves the body smoothly while maintaining collision physics
        --rb:SetTransform(targetPos, self:GetTransform().Rotation)

        -- OPTION B: For Dynamic Bodies (Physics-based)
        -- If you want it to push things out of the way, use velocity instead:
        local currentPos = Self:GetTransform().Translation
        local velocity = (targetPos - currentPos) * (1.0 / ts)
        rb:SetLinearVelocity(velocity)
    end
end

function OnClick()
end

function MyFun()
    print("This is the fun function!")
    RigidBody:ApplyImpulse(vec3.new(0, 100, 0), vec3.new(0, 0, 0))
end

function IncreaseSpeed()
    speed = speed + 1
end