incpos = 0
GL_LIGHT0 = 0x4000

-- BSP entities

mapSpawnPoints = {}
mapSpawnPointsSize = 0

mapPathCorners = {}
mapPathCornersSize = 0

mapTriggers = {}
mapTriggersSize = 0

mapTriggerTargets = {}
mapTriggerTargetsSize = 0

mapMovableParts = {}
mapMovablePartsSize = 0

mapAnimatedEntites = {}
mapAnimatedEntitesSize = 0

mapActiveAnimatedEntites = {}
mapActiveAnimatedEntitesId = 0




-- fields added to objects in lua
-- prevents garbage collection of objects created in lua functions
-- index format: <object name><fieldname> (eg.: Zombie3onPath)
customFields = {}

-- access custom fields through thesee funcitons
function _getfield(obj, fieldname)
	field = getfield(obj.nameP, fieldname)

	-- if the lua fields of the object has not been initialized
	if (field == nil) then
		obj:initFields()
		field = getfield(obj.nameP, fieldname)
	end

	return field
end

function _setfield(obj, fieldname, value)
	setfield(obj.nameP, fieldname, value)
end

function getfield(classname, fieldname)
	return customFields[classname .. fieldname]
end

function setfield(classname, fieldname, value)
	customFields[classname .. fieldname] = value
end


function log(str, inc)
	-- reseting indentication to the saved value
	if inc then
		if inc > 0 then
			incpos = getIndentNum()
		elseif inc == -13 then
			updateIndentNum(incpos)
		end
	end

	for i = 1, getIndentNum() do
		str = '\t' .. str
	end

	--print(str)
	logToConsole('' .. str)

	if inc then
		updateIndentNum(inc)
	end
end

function vec3:print()
	log('(' .. self.x .. ', ' .. self.y .. ', ' .. self.z .. ')')
end

function vec3:toString()
	return '(' .. self.x .. ', ' .. self.y .. ', ' .. self.z .. ')'
end

function strToVec3(str)
	vec = vec3(0,0,0)

	index = 0
	for w in string.gmatch(str, '%d+') do
		if index == 0 then
			vec.x = tonumber(w)
		elseif index == 1 then
			vec.y = tonumber(w)
		elseif index == 2 then
			vec.z = tonumber(w)
		end
		index = index + 1
	end

	return vec
end

function printtable(tableName)
	local result = {}
	for idx, value in pairs(tableName) do
		if not (idx == nil) then
			table.insert(result, idx .. '\n')
		end
	end

	result = table.concat(result)
	log(result)
end


function getMapElement(mapName, elementName)
	if mapName == 'textureDirectory' then
		return getTexture(elementName);
	elseif mapName == 'meshDirectory' then
		return getMesh(elementName)
	elseif mapName == 'shadedMeshDirectory' then
		return getShadedMesh(elementName)
	elseif mapName == "physicsMaterialDirectory" then
		return getPhysicsMaterial(elementName)
	elseif mapName == "physicsModelDirectory" then
		return getPhysicsModel(elementName)
	end
end


function registerEntity(entity)
	rootNode:addSP(entity)

	entityTableSize = entityTableSize + 1
	entityTable[entity.nameP] = entity
	
	if entity:isUnit() then
		unitTable[entity.nameP] = entity
		unitTableSize = unitTableSize + 1
	elseif entity:isItem() then
		itemTable[entity.nameP] = entity
		itemTableSize = itemTableSize + 1
	end
end

function registerEntityWithName(name, entity)
	rootNode:addSPWithName(entity, name, false)

	entityTableSize = entityTableSize + 1
	entityTable[name] = entity
	
	if entity:isUnit() then
		unitTable[name] = entity
		unitTableSize = unitTableSize + 1
	elseif entity:isItem() then
		itemTable[name] = entity
		itemTableSize = itemTableSize + 1
	end
end


function unregisterEntity(entity)
	rootNode:removeSP(entity)

	entityTableSize = entityTableSize - 1
	entityTable[entity.nameP] = nil
	
	if entity:isUnit() then
		unitTable[entity.nameP] = nil
		unitTableSize = unitTableSize - 1
	elseif entity:isItem() then
		itemTable[entity.nameP] = nil
		itemTableSize = itemTableSize - 1
	end
end



log('luaCommon.lua loaded successfully')
