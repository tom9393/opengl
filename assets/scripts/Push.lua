-- Push.lua

function OnCreate()
    print("Push script created!")
end

function OnUpdate(ts)
    if Input.IsKeyPressed(49) then
        self_rb:ApplyImpulse(vec3.new(0, 5, 0), vec3.new(0, 0, 0))
    end
end

function OnClick()
end