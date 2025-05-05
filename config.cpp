class CfgPatches
{
	class ZenAntiCombatLogout
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[] = { "DZ_Data" };
	};
};

class CfgMods
{
	class ZenAntiCombatLogout
	{
		dir="ZenAntiCombatLogout";
		picture=""; 
		action="";
		hideName=1;
		hidePicture=1;
		name="ZenAntiCombatLogout";
		credits=""; 
		author="Zenarchist";
		authorID="0";  
		version="1.0";
		extra=0;
		type="mod";
		dependencies[] =
		{
			"Game",
			"World",
			"Mission"
		};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = { "ZenAntiCombatLogout/Scripts/3_Game" };
			};
			class worldScriptModule
			{
				value = "";
				files[] = { "ZenAntiCombatLogout/scripts/4_world" };
			};
			class missionScriptModule
			{
				value = "";
				files[] = { "ZenAntiCombatLogout/scripts/5_mission" };
			};
		}
	};
};

class CfgVehicles
{
	// Define combat log flare
	class Roadflare;
	class Zen_CombatLogFlare : Roadflare
	{
		scope = 2;
		varQuantityDestroyOnMin = 1;
		class EnergyManager
		{
			energyAtSpawn = 200;
			energyUsagePerSecond = 1;
			updateInterval = 10;
			convertEnergyToQuantity = 1;
		};
		class NoiseRoadFlare
		{
			strength = 0;
			type = "";
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 9999;
				}
			}
		};
	};

	// Define ammo scanner
	class Inventory_Base;
	class Zen_CombatLogTrigger : Inventory_Base
	{
		scope = 1;
		model = "\dz\gear\consumables\Stone.p3d";
		hiddenSelections[] = { "zbytek" };
		hiddenSelectionsTextures[] = { "#(argb,8,8,3)color(1,1,1,0,CA)" };
	};
	class Zen_CombatLogExplosiveTrigger : Zen_CombatLogTrigger
	{
		scope = 1;
	};
};