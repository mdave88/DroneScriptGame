package.path = package.path .. ";../Resources/scripts/?.lua"
require('luaCommon')
require('gui')


-- CONSTANT VARIABLES
CORPSE_VISIBILITY_TIME = 40


-- --------------------------------------------------------------------------------------------
-- Lua classes
-- --------------------------------------------------------------------------------------------
-- have to make a wrapper class <- otherwise the c++ side will see it as a Unit instance only

class 'Medkit' (Item_wrapper)
function Medkit:__init(name, pos)
	Item_wrapper.__init(self, name, pos)
	self.pickable = true--false
	self:setPhysicalProperties('medkitShadedMesh', 'medkitPhysicsModel', pos)
end

function Medkit:onHitUnit(unit)
	if (0 < unit.healthP and unit.healthP < 100) then
		unit.healthP = unit.healthP + 50
		rootNode:removeSP(self)
		unit:addItemSP(self)
		self:deleteActor()
	end
end

function Medkit:onPickup()
	log('Medkit picked up')
end


class 'AssaultRifleAKS47' (Weapon_wrapper)
function AssaultRifleAKS47:__init()
	Weapon_wrapper.__init(self, 'AKS47')
	self:copyProperties(getWeapon('AKS47'))
end


class 'ZombieLord' (Unit_wrapper)
function ZombieLord:__init(pos, race)
	Unit_wrapper.__init(self, pos, race)
	self.healthP = 120
	self.raceP = 1
	log('new ZombieLord')
end

function ZombieLord:animate(dt)
	self:animate_def(dt)
end

function ZombieLord:AI(dt)
	self.animationNameP = 'zombie_idle'

	min = 10000
	minActor = 0

	for idx, entity in pairs(unitTable) do
		if entity:isUnit() and entity.healthP > 0 then
			if not (self.raceP == entity.raceP) then
				distance = self:getDistance(entity)

				if 0 < distance and distance < self.sightDistanceP * 2 then
					-- finding closest target
					if distance < min then
						min = distance
						minActor = entity
					end
				end

			end
		end
	end

	if min < 10000 then
		self.targetP = minActor
		if min < 15 then
			self.animationNameP = 'zombie_attack'

			animFrame = self.frameP
			if 10 < animFrame and animFrame < 12 then
				minActor:takeDamage(1)--(20)
			end
		else
			self.animationNameP = 'zombie_walk'

			-- move towards the enemy
			if self:faceTarget(dt, 0) then
				self:moveForward(dt)
			end
		end
	else
		self.targetP = nil
	end

end

-- --------------------------------------------------------------------------------------------
-- Lua defined methods
-- --------------------------------------------------------------------------------------------

function Door:interactL(entity)
	if entity:hasItem(self:getKey()) then
		self:open()
	else
		--play(nokeysound)
		--log('The unit havent got the right key.', 1)
	end
end

function Door:animateL(dt)
	if self.status == Door.OPENING then
		pos = self.posP
		pos.y = pos.y + 5.0 * dt
		self.posP= pos

		if pos.y >= 20 then
			self.status = Door.OPEN
		end
	elseif self.status == Door.CLOSING then
		pos = self.posP
		pos.y = pos.y - 10.0 * dt
		self.posP= pos

		if pos.y <= 0 then
			self.status = Door.CLOSED
		end
	end
end


function Unit_wrapper:animate(dt)
	self:animate_def(dt)
end

function Unit:AI()
	log('Unit:AI()')
end


function Zombie:initFields()
	_setfield(self, 'onPath', false)
	_setfield(self, 'currentPathTarget', nil)
	_setfield(self, 'currentPathElement', nil)
end

function Zombie:AI(dt)
	self.animationNameP = 'zombie_idle'
	min = 10000
	minActor = 0


	-- find enemy
	for idx, entity in pairs(unitTable) do
		if entity:isUnit() then
			if not (self.raceP == entity.raceP) and (entity.healthP > 0) then
				distance = self:getDistance(entity)
				
				if 0 < distance and distance < self.sightDistanceP then
					-- finding closest target
					if distance < min then
						min = distance
						minActor = entity
					end
				end
			end
		end
	end

	-- if hp is too low -> run away
	if min < 10000 and self.healthP < 50 then
		_setfield(self, 'onPath', false)

		self.targetP = minActor
		self.animationNameP = 'zombie_walk'

		-- move towards the enemy
		if self:faceTarget(dt, 180) then
			self:moveForward(dt)
		end
		do return end
	end

	-- set the animation
	if min < 10000 then
		_setfield(self, 'onPath', false)

		self.targetP = minActor

		-- if the enemy is close -> attack
		if min < 15 then
			self.animationNameP = 'zombie_attack'

			-- do damage on the right keyframe
			animFrame = self.frameP
			if 10 < animFrame and animFrame < 12 then
				minActor:takeDamage(10)
			end
		else
			-- if the enemy far away -> move towards it
			self.animationNameP = 'zombie_walk'

			-- move towards the enemy
			if self:faceTarget(dt, 0) then
				self:moveForward(dt)
			end
		end
	else

		-- no enemy found -> follow the closest path
		self.animationNameP = 'zombie_walk'

		self:followPath()
		if self:faceTarget(dt, 0) then
			self:moveForward(dt)
		end
	end
end


function Zombie:followPath()
	if (_getfield(self, 'onPath') == false) then
		-- find the closest path corner and start following the path
		closestCorner = findClosestBSPEnityFromTable(mapPathCorners, self)

		target = Entity(closestCorner.origin)
		self.targetP = target
		self.targetP.nameP = 'dummy'
		
		_setfield(self, 'onPath', true)
		_setfield(self, 'currentPathTarget', target)			-- have to save to prevent getting gc-ed
		_setfield(self, 'currentPathElement', closestCorner)
	else
		-- if the unit is close enough to our destination -> find the next path corner
		if self:getDistance(self.targetP) < 10 then
			nextCorner = mapPathCorners[ _getfield(self, 'currentPathElement'):getField('target') ]
			self.targetP.posP = nextCorner.origin

			_setfield(self, 'currentPathElement', nextCorner)
		end
	end
end

function Player:initL()
	-- spawnpoint
	--spawnpoint = mapSpawnPoints[2]

	self.posP = vec3(248, -18.9, 131)		--spawnpoint.pos
	--self.posP = vec3(210.7, 33.3, 48.7)	--spawnpoint.pos
	self.posP = spawnpoint.pos
	-- set angle
	rot = self.rotP
	--rot.y = spawnpoint.angle + 180
	self.rotP = rot

	-- weapon
	weaponAKS47 = AssaultRifleAKS47()
	self:addWeaponSP(weaponAKS47)
	self:setWeaponIndex(0)

	self:initController(true)

	--self:setPos(vec3(0, 20, 80))
	--bluekey = Item("bluekey")
	--self:addItemSP(bluekey)
end

function info()
	log('entites: ' .. entityTableSize .. '\n' .. 'units  : ' .. unitTableSize)
end

function animateSceneL(dt)
	-- animate lights
	offset = 0	--math.sin( getElapsedTime() / 1500.0 ) * 20.0;
	entityTable['light1'].posP = vec3(201, 8.0 + offset, 290)
	entityTable['light1'].rotP = vec3(10, getElapsedTime() / 10.0, 0)

	entityTable['light2'].posP = vec3(201, 8.0 + offset, 290)
	entityTable['light2'].rotP = vec3(10, getElapsedTime() / 10.0 + 180, 0)


	-- update units
	deadUnitsTable = {}

	for idx, entity in pairs(unitTable) do
		if entity:isUnit() and not entity:isPlayer() then
			if (entity.healthP <= 0) then
				if entity.deathTime == nil then					-- just got killed
					entity.deathTime = CORPSE_VISIBILITY_TIME
				elseif entity.deathTime > 0 then				-- count down
					entity.deathTime = entity.deathTime - dt
				else
					deadUnitsTable[entity.nameP] = entity
				end
			end
		end
	end

	-- unregister dead units and create new ones
	for idx, entity in pairs(deadUnitsTable) do
		unregisterEntity(entity)

		numZombieSpawnOnDeath = CONST_INT("Gameplay::Zombies::NumZombieSpawnOnDeath")
		maxEnemies = CONST_INT("Gameplay::NumMaxEnemies") - playerTableSize

		for i = 1, numZombieSpawnOnDeath do
			if (unitTableSize > maxEnemies) then break end

			spawnPointIndex = (unitTableSize - playerTableSize) % mapSpawnPointsSize
			spawnpoint = mapSpawnPoints[spawnPointIndex]

			-- add new zombie
			z = Zombie(spawnpoint.pos, 1)
			z:setShadedMeshByName('zombieShadedMesh')
			z:initController(false)
			z:setupRagdoll()
			registerEntity(z)
		end
	end
end

function bar()
	printtable(customFields)
	--unitTable['Zombie3']:followPath()
end


function kill_all()
	for idx, entity in pairs(unitTable) do
		if entity:isUnit() then
			if entity.raceP > 0 then
				entity:takeDamage(300)
			end
		end
	end
end


-- --------------------------------------------------------------------------------------------
-- BSP classes
-- --------------------------------------------------------------------------------------------

-- base class of animated entities
class 'BSPAnimatedEntity'
function BSPAnimatedEntity:__init(triggerId, modelId)
	self.triggerId = triggerId
	self.bspEntityData = mapTriggers[triggerId]
	self.modelId = modelId
	self.elapsedTime = 0

	self.animPos = vec3(0,0,0)
	self.animRot = vec3(0,0,0)

	self.angle = tonumber( mapTriggers[triggerId]:getField('angle') )
	self.team = mapTriggers[triggerId]:getField('team')

	self.bboxMin = mapMovableParts[self.modelId].bboxMin
	self.bboxMax = mapMovableParts[self.modelId].bboxMax

	local bboxDim = mapMovableParts[self.modelId].bboxDim

	self.moveDirection = vec3(0, 0, 1)

	-- up
	if self.angle == -1 then
		self.moveDirection = vec3(0, bboxDim.y, 0)
	-- down
	elseif self.angle == -2 then
		self.moveDirection = vec3(0, -bboxDim.y, 0)
	-- left
	elseif self.angle == 0 then
		self.moveDirection = vec3(-bboxDim.x, 0, 0)
	-- right
	elseif self.angle == 1 then
		self.moveDirection = vec3(bboxDim.x, 0, 0)
	-- by degrees
	elseif self.angle == 360 then
		self.moveDirection = vec3(bboxDim.x, 0, 0)
	elseif self.angle == 180 then
		self.moveDirection = vec3(-bboxDim.x, 0, 0)
	elseif self.angle == 90 then
		self.moveDirection = vec3(0, 0, -bboxDim.z)
	elseif self.angle == 270 then
		self.moveDirection = vec3(0, 0, bboxDim.z)
	end

	-- rotate in y axis
	self.targetOrigin = self.moveDirection / 0.05
	self.moveDirection:normalize()

	self.speed = 2.0
	if (not (mapTriggers[triggerId]:getField('speed') == '')) then
		self.speed = tonumber( mapTriggers[triggerId]:getField('speed') ) / 100.0
	end
end

function BSPAnimatedEntity:startAnimationL(triggerId)
	for idx, animTrigger in pairs(mapAnimatedEntites) do
		if (animTrigger.team == self.team) then
			mapActiveAnimatedEntites[ idx ] = mapAnimatedEntites[ idx ]
			mapMovableParts[animTrigger.modelId]:startAnimation()
		end
	end
end


-- BSPDoor:
class 'BSPDoor' (BSPAnimatedEntity)
function BSPDoor:__init(triggerId, modelId)
	BSPAnimatedEntity.__init(self, triggerId, modelId)

	self.doorType = 0
	if mapTriggers[triggerId]:getField('origin') == '' then
		self.doorType = 0	-- sliding door
	else
		self.doorType = 1	-- turning door
	end
end

function BSPDoor:animateL(dt)
	if (self.targetOrigin - self.animPos):length() > 0.1 then
		if self.doorType == 0 then
			self:slide(dt)
		else
			self:turn(dt)
		end
	else
		-- remove it from the list of animated entities
		mapMovableParts[self.modelId]:stopAnimation()
		table.remove(mapActiveAnimatedEntites, self.modelId)
	end
end

function BSPDoor:slide(dt)
	self.animPos = self.animPos + self.moveDirection * self.speed * dt
	mapMovableParts[self.modelId].animPos = self.animPos
end

function BSPDoor:turn(dt)
	self.animPos = self.animPos + self.moveDirection * self.speed * dt
	mapMovableParts[self.modelId].posP = self.animPos
end


-- BSP animate
function BSP:animateL(dt)
	for idx, entity in pairs(mapActiveAnimatedEntites) do
		entity:animateL(dt)
	end
end




log('classDefinitions.lua loaded successfully')

log('Lua definitions loaded.')
log('-----------------------')
