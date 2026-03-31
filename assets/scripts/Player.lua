-- Player.lua

lastMouseX = 0
lastMouseY = 0
firstMouse = true

function OnCreate()
    print("Player script initialized!")
end

function OnUpdate(ts)
    local mouseX, mouseY = Input.GetMousePosition()

    if firstMouse then
        lastMouseX = mouseX
        lastMouseY = mouseY
        firstMouse = false
    end

    local offsetX = mouseX - lastMouseX
    local offsetY = mouseY - lastMouseY

    lastMouseX = mouseX
    lastMouseY = mouseY

    local sensitivity = 0.0015
    offsetX = offsetX * sensitivity
    offsetY = offsetY * sensitivity

    Transform.Rotation.x = Transform.Rotation.x - offsetY
    Transform.Rotation.y = Transform.Rotation.y - offsetX


    local limit = Input.radians(89.0)
    if Transform.Rotation.x > limit then Transform.Rotation.x = limit end
    if Transform.Rotation.x < -limit then Transform.Rotation.x = -limit end

    local speed = 15.0
    local distance = speed * ts

    local forward = Transform:GetForward()
    local right = Transform:GetRight()

    if Input.IsKeyPressed(87) then
        Transform.Translation = Transform.Translation + (forward * distance)
    end
    if Input.IsKeyPressed(83) then
        Transform.Translation = Transform.Translation - (forward * distance)
    end
    if Input.IsKeyPressed(65) then
        Transform.Translation = Transform.Translation - (right * distance)
    end
    if Input.IsKeyPressed(68) then
        Transform.Translation = Transform.Translation + (right * distance)
    end
end

function OnClick()
end