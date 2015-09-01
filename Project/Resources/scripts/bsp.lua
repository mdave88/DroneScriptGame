package.path = package.path .. ";../Resources/scripts/?.lua"
require('classDefinitions')

-- the bsp map
gameMap = 0

numZombies = 0
maxNumZombies = 100


-- setup bsp map entities
-- we are interested in lights, spawn points, triggers, trigger targets
function setupEntity(bspEntity)
	classname = bspEntity:getField('classname')

	-- load path corners
	if (classname == 'path_corner') then
		mapPathCorners[bspEntity:getField('targetname')] = bspEntity
		mapPathCornersSize = mapPathCornersSize + 1
	end

	-- load triggers
	if (bspEntity.trigger == true) then
		mapTriggers[mapTriggersSize + 1] = bspEntity
		mapTriggersSize = mapTriggersSize + 1

		if (classname == 'func_door') then
			mapAnimatedEntites[ mapTriggersSize ] = BSPDoor(mapTriggersSize, bspEntity.modelId)
			mapAnimatedEntites[ bspEntity:getField('targetname') ] = mapAnimatedEntites[ mapTriggersSize ]
		end

		if (classname == 'func_button') then
			mapAnimatedEntites[ mapTriggersSize ] = BSPDoor(mapTriggersSize, bspEntity.modelId)
			--log(bspEntity:getFields())
		end
	end

	-- zombies
	if ((classname == 'CG_unit_zombie') and numZombies < maxNumZombies) then
		z = Zombie(bspEntity.origin + vec3(0, 10, 0), 1)
		z:setShadedMeshByName('zombieShadedMesh')
		z:setupRagdoll()
		z:initController(false)
		registerEntity(z)

		numZombies = numZombies + 1
	end

	-- zombie lords
	if (classname == 'CG_unit_zombielord') then
		z = ZombieLord(bspEntity.origin + vec3(0, 10, 0), 1)
		z:setShadedMeshByName('zombieLordShadedMesh')
		z:setupRagdoll()
		z:initController(false)
		registerEntity(z)
	end

	-- medkits
	if (classname == 'CG_item_medkit') then
		medkit = Medkit('medkit', bspEntity.origin + vec3(0, 10, 0))
		registerEntity(medkit)
	end
	

	-- load trigger targets
	if (not (classname:find('target_position') == nil)) or (not (classname:find('misc_teleporter_dest') == nil)) then
		pos = vec3(bspEntity.origin.x, bspEntity.origin.y, bspEntity.origin.z)
		pos.y = pos.y + 10

		angle = 0
		if not (bspEntity:getField('angle') == '') then
			angle = tonumber( bspEntity:getField('angle') )
		end

		targetposition = {
			pos = pos,
			angle = angle + 180
		}

		mapTriggerTargets[bspEntity:getField('targetname')] = targetposition
		mapTriggerTargetsSize = mapTriggerTargetsSize + 1
	end

	-- load target speakers
	if (bspEntity:getField('classname') == 'target_speaker') then
		--soundFileName = bspEntity:getField('noise')
		--soundEmitter = SoundEmitter(soundFileName, bspEntity.origin, vec3(0), true)
		--rootNode:addSP(soundEmitter)

		--soundEmitter:play()
	end

	-- load spawnpoints
	if (bspEntity:getField('classname') == 'info_player_deathmatch') then
		pos = vec3(bspEntity.origin.x, bspEntity.origin.y, bspEntity.origin.z)
		pos.y = pos.y + 10

		angle = 0
		if not (bspEntity:getField('angle') == '') then
			angle = tonumber( bspEntity:getField('angle') )
		end

		spawnpoint = {
			pos = pos,
			angle = angle + 180
		}

		mapSpawnPoints[mapSpawnPointsSize] = spawnpoint
		mapSpawnPointsSize = mapSpawnPointsSize + 1
	end

end


-- Prints the path the element is part of.
-- pathElement A part of the path.
function printPath(pathElement)
	local result = {}

	firstElement = pathElement		-- needed to prevent infinite loop
	table.insert(result, firstElement:getField('targetname') .. ' -> ')

	pathElement = mapPathCorners[ firstElement:getField('target') ]
	if ( not (pathElement == nil) ) then
		while ( not (pathElement:getField('target') == '') and not (pathElement:getField('targetname') == firstElement:getField('targetname')) ) do
			table.insert(result, pathElement:getField('targetname') .. ' -> ')

			pathElement = mapPathCorners[ pathElement:getField('target') ]
		end
	end

	if (pathElement:getField('targetname') == firstElement:getField('targetname')) then
		table.insert(result, pathElement:getField('targetname'))
	end

	result = table.concat(result)
	log(result)
end

-- Prints the path the element is part of.
-- pathElement A part of the path.
function printPathByElementName(pathElementName)
	firstElement = mapPathCorners[ pathElementName ]		-- needed to prevent infinite loop

	printPath(firstElement)
end

-- Prints the path the element is part of.
-- pathElement A part of the path.
function printPathByElementIndex(pathElementIndex)
	pathElementName = 'path_corner' .. pathElementIndex
	firstElement = mapPathCorners[ pathElementName ]		-- needed to prevent infinite loop
	printPath(firstElement)
end

-- print the paths
function printPaths()
	for idx, element in pairs(mapPathCorners) do
		printPath(element)
	end
end


function findClosestBSPEnityFromTable(bspEntityTable, gameEntity)
	closestBSPEntity = nil
	minDistance = 10000
	for idx, value in pairs(bspEntityTable) do
		distance = (value.origin - gameEntity.posP):length()
		if (distance < minDistance) then
			minDistance = distance
			closestBSPEntity = value
		end
	end

	return closestBSPEntity
end




-- --------------------------------------------------------------------------------------------
-- BSP triggers
-- --------------------------------------------------------------------------------------------

NX_TRIGGER_ON_ENTER = 1
NX_TRIGGER_ON_LEAVE = 2
NX_TRIGGER_ON_STAY  = 4
  
function trigger_teleport(status, actorName, triggerId)
	if status == NX_TRIGGER_ON_ENTER then
		targetName = mapTriggers[triggerId]:getField('target')
		entityTable[actorName].posP = mapTriggerTargets[targetName].pos

	elseif status == NX_TRIGGER_ON_STAY then

	elseif status == NX_TRIGGER_ON_LEAVE then

	end
end

function trigger_push(status, actorName, triggerId)
	--if (status == NX_TRIGGER_ON_ENTER) or (status == NX_TRIGGER_ON_LEAVE) then
	--if (status == NX_TRIGGER_ON_ENTER) or (status == NX_TRIGGER_ON_STAY) or (status == NX_TRIGGER_ON_LEAVE) then
		targetName = mapTriggers[triggerId]:getField('target')
		targetPos = mapTriggerTargets[targetName].pos

		-- the unit may be a dead one
		if (entityTable[actorName] ~= nil) then
			pushVel = (targetPos - entityTable[actorName].posP) * 0.68
			entityTable[actorName]:startJump(pushVel)
		end
		
	--end
end

function trigger_multiple(status, actorName, triggerId)
	
end

function func_button(status, actorName, triggerId)
	targetname = mapTriggers[ triggerId ]:getField('target')
	mapAnimatedEntites[ targetname ]:startAnimationL( targetname )
end


function func_door(status, actorName, triggerId)
	--if (status == NX_TRIGGER_ON_ENTER) or (status == NX_TRIGGER_ON_STAY) or (status == NX_TRIGGER_ON_LEAVE) then
		mapAnimatedEntites[ triggerId ]:startAnimationL(triggerId)
	--end
end

log('bsp.lua loaded successfully')
