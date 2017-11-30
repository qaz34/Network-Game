time = 0 

function Init() 
end 

function Destroy() 
end 

function Update(dt) 
time = time + dt 
local bulletSpeed = 100 
local xV, yV = GetVel() 
ApplyForce(yV * math.sin(time)*bulletSpeed * dt, yV * math.cos(time) * bulletSpeed * dt) 
end 
