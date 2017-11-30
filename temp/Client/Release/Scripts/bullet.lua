x = 0
function Init()
end

function Destroy()
end

function Update(dt)
local bulletSpeed = 200
x = x + dt * 10;
local xV, yV = GetVel()
ApplyForce(yV*bulletSpeed * dt, xV * bulletSpeed * dt) 
ApplyForce(math.sin(x) * 3, math.sin(x)* 3) 
end