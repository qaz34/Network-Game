time = 0 

function Init() 
end 

function Destroy() 
end 

function Update(dt) 
time = time + dt 
local bulletSpeed = 100 
local xV, yV = GetVel() 
ApplyForce(math.sin(xV * time) * bulletSpeed * dt, math.cos(yV * time) * bulletSpeed * dt) 
end 
