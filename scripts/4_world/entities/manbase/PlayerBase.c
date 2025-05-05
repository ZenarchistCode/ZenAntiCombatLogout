modded class PlayerBase
{
	private static int ANTI_COMBAT_PLAYER_ID_TRACKER = 0;
	private int m_AntiCombatPlayerID;
	private float m_CombatLogTimer = 0; // This is how many ms to wait before we can logout after engaging in combat
	private bool m_KillPlayerForCombatLogging = false; // Server-side flag - if true, player will be killed if they disconnect before timeout
	private int m_WillBePunishedForCombatLogging = 0; // Server & client flag - if true, logout message on client will reflect the severity of their situation
	private int m_DisableExitButtonSecs = 5; // How long to disable the exit button (client-side)

	void PlayerBase()
	{
		RegisterNetSyncVariableInt("m_AntiCombatPlayerID");
	}

	override void OnPlayerLoaded()
	{
		super.OnPlayerLoaded();

		ZenAntiCombatLogout_OnPlayerLoaded();
	}

	void ZenAntiCombatLogout_OnPlayerLoaded()
	{
		if (GetGame().IsDedicatedServer())
		{
			// This unique ID is for the client to tell the server who we shot at to trigger combat timer
			m_AntiCombatPlayerID = ANTI_COMBAT_PLAYER_ID_TRACKER;
			ANTI_COMBAT_PLAYER_ID_TRACKER++;
			SetSynchDirty();
		}
	}

	int GetAntiCombatPlayerID()
	{
		return m_AntiCombatPlayerID;
	}

	PlayerBase GetPlayerByCombatLogID(int id)
	{
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		
		foreach (Man playerMan : players)
		{
			PlayerBase player = PlayerBase.Cast(playerMan);
			if (player != NULL && player.GetAntiCombatPlayerID() == id)
			{
				return player;
			}
		}

		return NULL;
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

		// Client-side (client receives logout message info from server)
		if (rpc_type == ZEN_ANTI_COMBAT_LOG_MSG_RPC)
		{
			Param2<int, int> antiCombatLog_ClientParams;

			if (!ctx.Read(antiCombatLog_ClientParams))
				return;

			m_WillBePunishedForCombatLogging = antiCombatLog_ClientParams.param1;
			m_DisableExitButtonSecs = antiCombatLog_ClientParams.param2;
		}

		// Server-side (server receives notification that we shot at someone)
		if (rpc_type == ZEN_ANTI_COMBAT_LOG_RPC)
		{
			Param1<int> antiCombatLog_ServerParams;

			if (!ctx.Read(antiCombatLog_ServerParams))
				return;

			// Reset our combat logout timer too.
			int highBits, lowBits;
			GetGame().GetPlayerNetworkIDByIdentityID(sender.GetPlayerId(), lowBits, highBits);
			PlayerBase shooter = PlayerBase.Cast(GetGame().GetObjectByNetworkId(lowBits, highBits));
			PlayerBase victim = GetPlayerByCombatLogID(antiCombatLog_ServerParams.param1);

			// If victim exists and they're not dead, reset their combat logout timer.
			if (victim && victim.GetIdentity() && victim.IsAlive())
			{
				victim.SetCombatLogTimer(shooter, victim);
			}

			if (shooter && shooter.IsAlive())
			{
				shooter.SetCombatLogTimer(shooter, victim);
			}
		}
    }

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		// Separate my method to allow easy overriding
		HandleAntiCombatLog(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
	}

	void HandleAntiCombatLog(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		PlayerBase attacker;

		// If this player got hit by vehicle, trigger combat log timer for driver
		CarScript vehicle = CarScript.Cast(source);
		if (vehicle)
		{
			attacker = PlayerBase.Cast(vehicle.CrewMember(0));
			if (attacker)
			{
				float speed = 0.0;

				Car car;
				if (Class.CastTo(car, vehicle))
					speed = car.GetSpeedometer();
				else
					speed = GetVelocity(vehicle).Length() * 3.6;

				if (speed > 1 && damageResult && damageResult.GetDamage(dmgZone, "Health") > 1)
				{
					attacker.SetCombatLogTimer(attacker, this);
					return; // No need to continue.
				}
			}
		}

		// Don't trigger combat log timer for dead players
		if (!IsAlive())
			return;

		attacker = PlayerBase.Cast(source.GetHierarchyRootPlayer());

		// If we were not attacked by another player, don't set our combat timer
		if (!attacker || attacker == this)
			return;

		attacker.SetCombatLogTimer(attacker, this);
		this.SetCombatLogTimer(attacker, this);
	}

	// Get exit button disabled secs
	int GetDisableExitButtonSecs()
	{
		return m_DisableExitButtonSecs;
	}

	// Inform player they will be killed for combat logging
	void InformPlayerOfCombatLogout(int willBeKilled)
	{
		Param2<int, int> params = new Param2<int, int>(willBeKilled, GetZenAntiCombatLogoutConfig().DisableExitButtonSecs);
		GetGame().RPCSingleParam(this, ZEN_ANTI_COMBAT_LOG_MSG_RPC, params, true, GetIdentity());
		m_WillBePunishedForCombatLogging = willBeKilled;
	}

	// Check if player will be killed or a flare dropped for combat logging
	int WillBePunishedForCombatLogging()
	{
		return m_WillBePunishedForCombatLogging;
	}

	// Stores who shot at who first to track the aggressor in combat <SteamID, AggressorStatus>
	ref map<string, bool> m_ShotAtUsFirst = new ref map<string, bool>;

	// Reset combat log timer
	void SetCombatLogTimer(PlayerBase attacker = NULL, PlayerBase victim = NULL)
	{
		// Do we have both a victim and an attacker in this altercation?
		if (attacker != victim && victim != NULL && attacker != NULL && victim.GetIdentity() != NULL && attacker.GetIdentity() != NULL)
		{
			//! On first instance of assault, attacker is presumed the aggressor, and victim is presumed innocent. 
			//! Not always the case but whatever. Not gonna track who aimed at who first or who talked shit first.
			//! Mainly used for situations where 'estimating' who started the fight is ok, like integrations with ExpansionAI guards.
			if (!victim.m_ShotAtUsFirst.Contains(attacker.GetCachedID())) 
			{
				attacker.m_ShotAtUsFirst.Set(victim.GetCachedID(), false);
				victim.m_ShotAtUsFirst.Set(attacker.GetCachedID(), true);
			}
		}

		m_CombatLogTimer = GetGame().GetTime() + (GetZenAntiCombatLogoutConfig().CombatLogoutSecs * 1000);
	}

	// Check if we started combat with the given player
	bool DidWeStartCombatWith(notnull PlayerBase enemy)
	{
		bool weShotFirst = false;
		enemy.m_ShotAtUsFirst.Find(GetCachedID(), weShotFirst);
		return weShotFirst;
	}

	// Resets our combat log timer
	void ResetCombatLogTimer()
	{
		m_CombatLogTimer = 0;
	}

	// Get the current combat logout timer (resets whenever damage is dealt or a shot is fired at us)
	float GetCombatLogTimer()
	{
		return m_CombatLogTimer;
	}

	// (Client-side) Sends an RPC to the server notifying it that we shot at some poor fucker
	void InformServerThatWeShotAt(notnull PlayerBase player)
	{
		auto params = new Param1<int>(player.GetAntiCombatPlayerID());
		GetGame().RPCSingleParam(this, ZEN_ANTI_COMBAT_LOG_RPC, params, true);
	}
}