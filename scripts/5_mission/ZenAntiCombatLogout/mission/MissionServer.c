modded class MissionServer
{
	// Used to store players who have clicked "Exit" and begun logout, but haven't yet disconnected (allows us to send them combat logout info before actual disconnect)
	private ref map<PlayerBase, bool> m_ZenLogoutQueueCombatInital;
	private bool m_ZenAntiCombatLogEnabledCache;

	// Called when server initializes
	override void OnInit()
	{
		super.OnInit();
		
		m_ZenAntiCombatLogEnabledCache = false;

		#ifdef ZenModPack
		if (!ZenModEnabled("ZenAntiCombatLogout"))
			return;
		
		ZenModPackConfig.OnCfgChanged.Insert(RefreshZenAntiCombatLogoutState);
		#endif

		Print("[ZenAntiCombatLogout] OnInit");
		m_ZenAntiCombatLogEnabledCache = true;

		// Load config
		GetZenAntiCombatLogoutConfig();

		// Prepare logout queue
		m_ZenLogoutQueueCombatInital = new map<PlayerBase, bool>;

		#ifdef ZenDiscordAPI
		bool webhooksEnabled = false;
		if (GetZenAntiCombatLogoutConfig().DiscordWebhooks.Count() > 0 && !GetZenAntiCombatLogoutConfig().DiscordWebhooks.Get(0).Contains("OnlyWorksIfZenDiscordAPIPresent"))
		{
			webhooksEnabled = true;
		}

		Print("[ZenAntiCombatLogout] ZenDiscordAPI detected - webhooks enabled=" + webhooksEnabled);
		#endif
	}

	// Detect logout/disconnect event - check if player's combat timer has been set, if so, override default logout time
	override void OnClientDisconnectedEvent(PlayerIdentity identity, PlayerBase player, int logoutTime, bool authFailed)
	{
		if (!m_ZenAntiCombatLogEnabledCache)
		{
			super.OnClientDisconnectedEvent(identity, player, logoutTime, authFailed);
			return;
		}

		int origTime = logoutTime;

		// Check if player is alive and their combat logout timer is still running
		if (g_Game && player && player.GetCombatLogTimer() > 0 && player.IsAlive())
		{
			if (g_Game.GetTime() > player.GetCombatLogTimer())
			{
				// Set combat log timer to 0
				player.ResetCombatLogTimer();
			}
			else
			{
				// Player's logout timer is still active, override global.xml logout time setting
				logoutTime = (player.GetCombatLogTimer() - g_Game.GetTime()) / 1000;
				if (logoutTime <= origTime)
				{
					logoutTime = origTime;
				}
			}
		}

		// Pass new logoutTime to super()
		super.OnClientDisconnectedEvent(identity, player, logoutTime, authFailed);
	}

	// Detect force-disconnect (Alt+F4 or clicked Exit button)
    override void OnEvent(EventType eventTypeId, Param params)
    {
        super.OnEvent(eventTypeId, params);
		
		if (!m_ZenAntiCombatLogEnabledCache)
	        return;
	
	    if (eventTypeId != ClientDisconnectedEventTypeID && eventTypeId != LogoutCancelEventTypeID)
	        return;
	
	    if (!m_ZenLogoutQueueCombatInital)
	        return;

        PlayerIdentity identity;
        PlayerBase player;

		// Disconnect event (called when player clicks Exit to logout, and again when player actually leaves server)
		if (eventTypeId == ClientDisconnectedEventTypeID)
        {
			// Get disconnect parameters so we can identify player and check their combat log status
			ClientDisconnectedEventParams discoParams;
			if (!Class.CastTo(discoParams, params))
				return;

			identity = discoParams.param1;
			player = PlayerBase.Cast(discoParams.param2);
			bool logoutTimeExpired = IsZenCombatLogoutTimeExpired(player);

			// This if statement is executed when player initiates logout procedure (ie. clicks Exit button from game menu)
			if (player && (!m_ZenLogoutQueueCombatInital.Get(player) || !m_ZenLogoutQueueCombatInital.Contains(player)))
			{
				// Store to initial player logout queue so we can detect early disconnects (no other way to tell a regular disconnect from an early one otherwise)
				m_ZenLogoutQueueCombatInital.Insert(player, true);

				// Inform player they will be killed if they combat log
				if (player.IsAlive() && GetZenAntiCombatLogoutConfig().NotifyPlayerOfPenalty)
				{
					if (logoutTimeExpired) // Logout timer has expired, player is fine
					{
						player.InformPlayerOfCombatLogout(0); // No logout notification (use vanilla text)
					}
					else
					if (GetZenAntiCombatLogoutConfig().KillPlayer > 0) // Player gon' die
					{
						player.InformPlayerOfCombatLogout(1);
					}
					else
					if (GetZenAntiCombatLogoutConfig().DropFlareOnPlayer == 1) // Player gon' be exposed
					{
						player.InformPlayerOfCombatLogout(2);
					}
					else
					{
						player.InformPlayerOfCombatLogout(3); // If notify player is on and no penalty, inform player why logout timer is extended
					}
				}
			} else
			if (!logoutTimeExpired) // This is executed when player actually disconnects
			{
				if (player)
				{
					m_ZenLogoutQueueCombatInital.Remove(player);

					// if player is not dead, handle their penalties (if any)
					if (player.IsAlive())
					{
						if (GetZenAntiCombatLogoutConfig().DropFlareOnPlayer == 1)
						{
							// Spawn flare on logged player
							Zen_CombatLogFlare flare = Zen_CombatLogFlare.Cast(g_Game.CreateObjectEx("Zen_CombatLogFlare", player.GetPosition(), ECE_PLACE_ON_SURFACE));
							flare.SetOrientation(player.GetOrientation());
						}
						
						if (GetZenAntiCombatLogoutConfig().KillPlayer == 2)
						{
							// Kill player immediately
							player.SetHealth(0);
						}

						string printMsg = player.GetCachedName() + " " + player.GetCachedID() + " MAY have attempted to combat log, HP=" + player.GetHealth() + ". Their combat logout timer is still running (" + (player.GetCombatLogTimer() / 1000) + "s) during disconnection @ " + player.GetPosition();
						Print("[ZenAntiCombatLogout] " + printMsg);

						if (GetZenAntiCombatLogoutConfig().DiscordWebhooks.Count() > 0 && !GetZenAntiCombatLogoutConfig().DiscordWebhooks.Get(0).Contains("OnlyWorksIfZenDiscordAPIPresent"))
						{
							string title = "ZenAntiCombatLogout | " + g_Game.GetWorldName();
							ZenDiscordMessage msg = new ZenDiscordMessage(title);
							msg.SetTitle(title);
							msg.SetMessage(printMsg);
							msg.SetColor(255, 160, 0);
							msg.AddWebhooks(GetZenAntiCombatLogoutConfig().DiscordWebhooks);
							GetZenDiscordAPI().SendMessage(msg);
						}
					}
				}
			}
        }
		else
		if (eventTypeId == LogoutCancelEventTypeID) // Cancel logout
		{
			LogoutCancelEventParams logoutCancelParams;
			if (!Class.CastTo(logoutCancelParams, params))
				return;

			if (!Class.CastTo(player, logoutCancelParams.param1))
				return;

			// Player canceled logout, remove them from logout queue
			if (player)
				m_ZenLogoutQueueCombatInital.Remove(player);
		}
    }
	
	// Triggered if ZenModConfig changes during live session via !commands
	void RefreshZenAntiCombatLogoutState(ZenConfigBase cfg)
	{
		#ifdef ZenModPack
		if (cfg.Type() != ZenModPackConfig)
			return;
		
		m_ZenAntiCombatLogEnabledCache = ZenModEnabled("ZenAntiCombatLogout");
		#else 
		return;
		#endif
		
		Print("[ZenAntiCombatLogout] RefreshZenAntiCombatLogoutState=" + m_ZenAntiCombatLogEnabledCache);

		if (m_ZenAntiCombatLogEnabledCache)
		{
			GetZenAntiCombatLogoutConfig();

			if (!m_ZenLogoutQueueCombatInital)
				m_ZenLogoutQueueCombatInital = new map<PlayerBase, bool>;
		}
		else
		{
			if (m_ZenLogoutQueueCombatInital)
				m_ZenLogoutQueueCombatInital.Clear();
		}
	}
	
	// If the player has officially disconnected, remove them from our pre-logout queue
	override void InvokeOnDisconnect(PlayerBase player)
	{
		super.InvokeOnDisconnect(player);

		if (!m_ZenAntiCombatLogEnabledCache)
	        return;

		if (player)
			m_ZenLogoutQueueCombatInital.Remove(player);
	}

	// Check if combat logout timer has expired for given player
	bool IsZenCombatLogoutTimeExpired(PlayerBase player)
	{
		if (!player)
			return true;

		// If combat log timer == 0 then they have not engaged in combat recently
		if (player.GetCombatLogTimer() == 0)
			return true;

		LogoutInfo params = m_LogoutPlayers.Get(player);
		if (!params)
			params = m_NewLogoutPlayers.Get(player);

		// If game time <= LogoutInfo's logout time, then player is exiting before their combat log timer expires (so return false)
		return params && g_Game.GetTime() > params.param1;
	}

	// Don't penalize player for combat logging due to server restart/shutdown/admin kick etc. Otherwise will mass murder everyone who had a fight right before a restart
	override bool ShouldPlayerBeKilled(PlayerBase player)
	{
		if (!m_ZenAntiCombatLogEnabledCache)
	        return super.ShouldPlayerBeKilled(player);

		bool killPlayer = super.ShouldPlayerBeKilled(player);

		if (killPlayer)
			return true;

		// If player has been flagged to die for combat logging and their logout timer has not expired, fuck em
		if (player.WillBePunishedForCombatLogging() == 1 && !IsZenCombatLogoutTimeExpired(player))
		{
			// If player was kicked off for a server restart, don't kill them as that's going to kill pretty everyone who was in a gunfight leading up to the restart
			switch (player.GetKickOffReason())
			{
				case EClientKicked.SERVER_EXIT:
					killPlayer = false;
					break;
				case EClientKicked.KICK_ALL_ADMIN:
					killPlayer = false;
					break;
				case EClientKicked.KICK_ALL_SERVER:
					killPlayer = false;
					break;
				case EClientKicked.SERVER_SHUTDOWN:
					killPlayer = false;
					break;
				default:
					killPlayer = true;
			}
		}

		// If we're killing the player, and server config is set to only drop flares on dead players, then drop the flare
		if (killPlayer && player.IsAlive() && GetZenAntiCombatLogoutConfig().DropFlareOnPlayer == 2)
		{
			Zen_CombatLogFlare flare = Zen_CombatLogFlare.Cast(g_Game.CreateObjectEx("Zen_CombatLogFlare", player.GetPosition(), ECE_PLACE_ON_SURFACE));
			flare.SetOrientation(player.GetOrientation());
		}

		if (player && player.GetIdentity() && killPlayer)
			Print("[ZenAntiCombatLogout] Player " + player.GetCachedID() + " killed for combat logging.");

		return killPlayer;
	}
}