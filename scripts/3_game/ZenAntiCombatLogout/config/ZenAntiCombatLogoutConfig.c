ref ZenAntiCombatLogoutConfig g_ZenAntiCombatLogout;

static ZenAntiCombatLogoutConfig GetZenAntiCombatLogoutConfig()
{
	if (!g_ZenAntiCombatLogout)
		GetZenConfigRegister().RegisterConfig(ZenAntiCombatLogoutConfig);

	return g_ZenAntiCombatLogout;
}

modded class ZenConfigRegister
{
	override void RegisterPreload()
	{
		super.RegisterPreload();
		RegisterType(ZenAntiCombatLogoutConfig); // auto-create + auto-load
	}
}

class ZenAntiCombatLogoutConfig: ZenConfigBase
{
	// -------------------------
	// CONFIG SETTINGS
	// -------------------------
	override void OnRegistered()
	{
		g_ZenAntiCombatLogout = this;
	}
	
	override string    	GetCurrentVersion()   		{ return "1.29.1"; }
	override bool		ShouldLoadOnServer() 		{ return true; }
	
	override bool ReadJson(string path, out string err)
	{
		return JsonFileLoader<ZenAntiCombatLogoutConfig>.LoadFile(path, this, err);
	}

	override bool WriteJson(string path, out string err)
	{
		return JsonFileLoader<ZenAntiCombatLogoutConfig>.SaveFile(path, this, err);
	}

	// -------------------------
	// CONFIG VARIABLES
	// -------------------------

	// Config data
	int CombatLogoutSecs = 300; // How many seconds to keep player on server after engaging in combat (shooting at player, meleeing them or grenading them if TriggerOnExplosiveRadius > 0)
	int DisableExitButtonSecs = 5; // How long to disable the Exit button when logging out after combat
	bool NotifyPlayerOfPenalty = true; // Whether or not to notify the player of the penalty for combat logging
	bool TriggerOnGunshot = false; // Whether or not to trigger the combat log timer for simply firing your gun (hunting, raiding etc)
	bool TriggerForExpansionAI = false; // Whether or not to trigger combat log timer on AI
	int TriggerOnBulletImpactRadius = 0; // // Experimental (set to 0 to disable) - Radius around bullet impact to detect and flag players for engaging in combat
	int TriggerOnExplosiveRadius = 30; // Radius around explosions to detect and flag players for engaging in combat (set to 0 to disable)
	int DropFlareOnPlayer = 0; // Whether or not to drop a flare on a player (0 = disabled, 1 = when they logout before timer, 2 = when they are killed for combat logging)
	int KillPlayer = 0; // Whether or not to kill a player for leaving the server before the combat log timer runs out
	ref array<string> DiscordWebhooks = new array<string>;

	// Load config file or create default file if config doesn't exsit
	override void SetDefaults()
	{
		DiscordWebhooks.Insert("InsertDiscordWebhooks");
	}
}