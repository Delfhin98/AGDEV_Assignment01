#include "LuaInterface.h"
#include <iostream>
#include <stdarg.h>
using namespace std;

// Allocating and initializing CLuaInterface's static data member.  
// The pointer is allocated but not the object's constructor.
CLuaInterface *CLuaInterface::s_instance = 0;

CLuaInterface::CLuaInterface()
	: theLuaState(NULL)
{
}


CLuaInterface::~CLuaInterface()
{
}


// Initialisation of the Lua Interface Class
bool CLuaInterface::Init()
{
	bool result = false;

	// 1. Create lua state
	theLuaState = lua_open();

	if (theLuaState)
	{
		// 2. Load lua auxiliary libraries
		luaL_openlibs(theLuaState);

		// 3. Load lua script
		luaL_dofile(theLuaState, "Image//DM2240.lua");

		result = true;
	}

	theErrorState = lua_open();

	if ((theLuaState) && (theErrorState))
	{
		// 2. Load lua auxiliary libraries
		luaL_openlibs(theLuaState);

		// Load the error lua script
		luaL_dofile(theErrorState, "Image//errorLookup.lua");
	}

	return result;
}

// Run the Lua Interface Class
void CLuaInterface::Run()
{
	if (theLuaState == NULL)
		return;

	// 4. Read the variables
	// lua_getglobal(lua_State*, const char*)
	lua_getglobal(theLuaState, "title");
	// extract value, index -1 as variable is already retrieved using lua_getglobal
	const char *title = lua_tostring(theLuaState, -1);

	lua_getglobal(theLuaState, "width");
	int width = lua_tointeger(theLuaState, -1);

	lua_getglobal(theLuaState, "height");
	int height = lua_tointeger(theLuaState, -1);

	// Display on screen
	cout << title << endl;
	cout << "---------------------------------------" << endl;
	cout << "Width of screen : " << width << endl;
	cout << "Height of screen : " << height << endl;
}

// Get an integer value through the Lua Interface Class
int CLuaInterface::getIntValue(const char* varName)
{
	lua_getglobal(theLuaState, varName);
	return lua_tointeger(theLuaState, -1);
}
// Get a float value through the Lua Interface Class
float CLuaInterface::getFloatValue(const char* varName)
{
	lua_getglobal(theLuaState, varName);
	return (float)lua_tonumber(theLuaState, -1);
}

// Get a char value through the Lua Interface Class
char CLuaInterface::getCharValue(const char* varName)
{
	lua_getglobal(theLuaState, varName);

	size_t len;
	const char* cstr = lua_tolstring(theLuaState, -1, &len);
	// if the string is not empty, then return the first char
	if (len > 0)
		return cstr[0];
	// else return a default value of <SPACE>
	else
		return ' ';
}

// Get Vector3 values through the Lua Interface Class
Vector3 CLuaInterface::getVector3Values(const char* varName)
{
	lua_getglobal(theLuaState, varName);
	lua_rawgeti(theLuaState, -1, 1);
	int x = lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);
	lua_rawgeti(theLuaState, -1, 2);
	int y = lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);
	lua_rawgeti(theLuaState, -1, 3);
	int z = lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);

	return Vector3(x, y, z);
}

// Get distance square value through the Lua Interface Class
float CLuaInterface::getDistanceSquareValue(const char* varName,
											Vector3 source, 
											Vector3 destination)
{
	lua_getglobal(theLuaState, varName);
	lua_pushnumber(theLuaState, source.x);
	lua_pushnumber(theLuaState, source.y);
	lua_pushnumber(theLuaState, source.z);
	lua_pushnumber(theLuaState, destination.x);
	lua_pushnumber(theLuaState, destination.y);
	lua_pushnumber(theLuaState, destination.z);
	lua_call(theLuaState, 6, 1);
	float distanceSquare = (float)lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);
	return distanceSquare;
}

// Get variable number of values through the Lua Interface Class
int CLuaInterface::getVariableValues(const char* varName, int &a, int &b, int &c, int &d)
{
	lua_getglobal(theLuaState, varName);
	lua_pushnumber(theLuaState, 1000);
	lua_pushnumber(theLuaState, 2000);
	lua_pushnumber(theLuaState, 3000);
	lua_pushnumber(theLuaState, 4000);
	lua_call(theLuaState, 4, 4);
	a = lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);
	b = lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);
	c = lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);
	d = lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);

	return true;
}

// Save an integer value through the Lua Interface Class
void CLuaInterface::saveIntValue(const char* varName, const int value, const bool bOverwrite)
{
	lua_getglobal(theLuaState, "SaveToLuaFile");
	char outputString[80];
	sprintf(outputString, "%s = %d\n", varName, value);
	lua_pushstring(theLuaState, outputString);
	lua_pushinteger(theLuaState, bOverwrite);
	lua_call(theLuaState, 2, 0); cout << "....................";
}
// Save a float value through the Lua Interface Class
void CLuaInterface::saveFloatValue(const char* varName, const float value, const bool bOverwrite)
{
	lua_getglobal(theLuaState, "SaveToLuaFile");
	char outputString[80];
	sprintf(outputString, "%s = %6.4f\n", varName, value);
	lua_pushstring(theLuaState, outputString);
	lua_pushinteger(theLuaState, bOverwrite);
	lua_call(theLuaState, 2, 0);
}

// Drop the Lua Interface Class
void CLuaInterface::Drop()
{
	if (theLuaState)
	{
		// Close lua state
		lua_close(theLuaState);
	}
}

// Extract a field from a table
float CLuaInterface::GetField(const char *key) 
{
	int result = false;

	// Check if the variables in the Lua stack belongs to a table
	if (!lua_istable(theLuaState, -1))
		error("error100");

	lua_pushstring(theLuaState, key);
	lua_gettable(theLuaState, -2);
	if (!lua_isnumber(theLuaState, -1))
		error("error101");
	result = (int)lua_tonumber(theLuaState, -1);
	lua_pop(theLuaState, 1);  /* remove number */
	return result;
}

// Get error message using an error code
void CLuaInterface::error(const char *errorCode)
{
	if (theErrorState == NULL)
		return;

	lua_getglobal(theErrorState, errorCode);
	const char *errorMsg = lua_tostring(theErrorState, -1);
	if (errorMsg != NULL)
		cout << errorMsg << endl;
	else
		cout << errorCode << " is not valid.\n*** Please contact the developer ***" << endl;
}