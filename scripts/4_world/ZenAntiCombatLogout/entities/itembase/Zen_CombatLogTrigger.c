// This "object" is invisible and only exists to scan the nearby area for players (used to detect bullet & explosive impacts near players)
class Zen_CombatLogTrigger extends Inventory_Base
{
	void Zen_CombatLogTrigger()
	{
		// Delete immediately
		g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(DeleteSafe, 1, false);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);

		#ifdef SERVER
		// Get radius config
		int radius = 0;
		
		if (GetType().Contains("Explosive")) // Zen_CombatLogExplosiveTrigger (ie. M79 grenade launcher impact)
			radius = GetZenAntiCombatLogoutConfig().TriggerOnExplosiveRadius;
		else
			radius = GetZenAntiCombatLogoutConfig().TriggerOnBulletImpactRadius;

		// Check if config is disabled
		if (radius <= 0)
			return;

		vector ourRealPos = GetPosition();
		ourRealPos[1] = g_Game.SurfaceY(ourRealPos[0], ourRealPos[2]);

		// Scan player list and check distance
		array<Man> players = new array<Man>;
		g_Game.GetWorld().GetPlayerList(players);
		PlayerBase pb;
		for (int x = 0; x < players.Count(); x++)
		{
			if (vector.Distance(players.Get(x).GetPosition(), ourRealPos) < radius)
			{
				pb = PlayerBase.Cast(players.Get(x));
				pb.SetCombatLogTimer();
			}
		}
		#endif
	}
};

// Used to differentiate between regular ammo and explosive ammo
class Zen_CombatLogExplosiveTrigger extends Zen_CombatLogTrigger {};