// Detect bullet impacts
modded class DayZGame
{
	override void FirearmEffects(Object source, Object directHit, int componentIndex, string surface, vector pos, vector surfNormal, vector exitPos, vector inSpeed, vector outSpeed, bool isWater, bool deflected, string ammoType)
	{
		super.FirearmEffects(source, directHit, componentIndex, surface, pos, surfNormal, exitPos, inSpeed, outSpeed, isWater, deflected, ammoType);

		#ifdef SERVER
		// If bullet impact detect on server, spawn combat log triggers (scans area for players)
		// Spawn 100m in the air away from explosion, or our "invisible" rock object's hitbox will block damage dispersion (height is adjusted in Zen_CombatLogTrigger.c)
		vector spawnPos = pos + "0 100 0";
		if ((ammoType == "Bullet_40mm_Explosive" || ammoType == "AType_Bullet_40mm_ChemGas") && GetZenAntiCombatLogoutConfig().TriggerOnExplosiveRadius > 0)
		{
			g_Game.CreateObjectEx("Zen_CombatLogExplosiveTrigger", spawnPos, ECE_KEEPHEIGHT);
			Print("[ZenAntiCombatLogout] Spawned Zen_CombatLogExplosiveTrigger @ " + spawnPos + " for explosive type " + ammoType + " @ " + pos);
		}	
		else
		if (GetZenAntiCombatLogoutConfig().TriggerOnBulletImpactRadius > 0)
		{
			g_Game.CreateObjectEx("Zen_CombatLogTrigger", spawnPos, ECE_KEEPHEIGHT);
			Print("[ZenAntiCombatLogout] Spawned Zen_CombatLogTrigger @ " + spawnPos + " for explosive type " + ammoType + " @ " + pos);
		}
		#endif
	}
}