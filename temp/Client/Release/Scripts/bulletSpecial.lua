
function Init()
end

function Destroy()
end

function Update(dt)
local bulletSpeed = 600
local xV, yV = GetVel()
ApplyForce(yV*bulletSpeed * dt, xV * bulletSpeed * dt) 
end