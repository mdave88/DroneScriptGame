package.path = package.path .. ";../Resources/scripts/?.lua"
require('classDefinitions')
require('bsp')


--for i=1,0 do
	--z = ZombieLord(vec3(30 - i * 10, 10, -70), 1)
	--z:setShadedMeshByName('zombieLordShadedMesh')
	--z:setupRagdoll()
	--z:initController(false)
	--registerEntity(z)
--end
--
--for i=1,2 do
	----z = Zombie(vec3(30 - i * 10, 10, -40), 1)
	--z = Zombie(vec3(30 - i * 10, 10, 60), 1)
	--z:setShadedMeshByName('zombieShadedMesh')	--z:setShadedMesh(getShadedMesh('zombieShadedMesh'))	--z:setShadedMesh(getMapElement('shadedMeshDirectory', 'zombieShadedMesh'))
	--z:setupRagdoll()
	--z:initController(false)
	--registerEntity(z)
--end


--medkit = Medkit('medkit', vec3(0, 10, -30))
--registerEntity(medkit)

--medkit2 = Medkit('medkit1', vec3(0, 10, 10))
--registerEntity(medkit2)

--medkit3 = Medkit('medkit2', vec3(0, 20, 60))
--registerEntity(medkit3)

--medkit4 = Medkit('medkit3', vec3(0, 30, 60))
--registerEntity(medkit4)


-- Initialize lights

--LightSource.setLightModel(vec3(0.01f, 0.01f, 0.01f), 1.0f)


light0 = LightSource(0)
light1 = LightSource(1)
light2 = LightSource(2)


light0:setBase(vec3(1.5), vec3(0.5, 0.5, 0.1), vec3(0.0), vec3(0.0, 1.0, 0.0))
light0:setAttenuation(vec3(0.0, 0.0, 0.005))
--light0:setAttenuation(vec3(0.0, 0.0, 0.01))
light0:setVisibility(true)

light1:setBase(vec3(0.0), vec3(0.8, 0.8, 0.8), vec3(0.0), vec3(0, 20.0, -60));
light1:setAttenuation(vec3(0.0, 0.0, 0.0005))
light1.type = LightSource.LIGHTTYPE_SPOT
light1:setVisibility(true)

light2:setBase(vec3(0.0), vec3(0.1, 0.8, 0.1), vec3(0.0), vec3(-10, 20.0, -50))
light2:setAttenuation(vec3(0.0, 0.0, 0.0005))
light2.type = LightSource.LIGHTTYPE_SPOT
light2:setVisibility(true)

registerEntityWithName('light0', light0)
registerEntityWithName('light1', light1)
registerEntityWithName('light2', light2)


math.randomseed( os.time() )

log('init.lua loaded successfully')
