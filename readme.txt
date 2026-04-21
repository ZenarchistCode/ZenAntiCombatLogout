What Is This?

This mod adds automated combat-logout detection and prevention functionality to the server.

I got tired of dealing with reports of combat logging on my servers, so I decided to make an automatic solution. It's not perfect, but for me it's better than nothing.

How It Works:

Once a player is flagged as being engaged in combat, their logout timer is increased significantly (5 minutes by default). 

If they exit the game before their logout timer expires, their character will stay on the server until the timer expires, giving other players time to find them before their character disappears.

You can also apply various penalties in the JSON config which I'll explain below.

Whenever a player shoots at another player, or deals any kind of melee damage, both players will be flagged as being engaged in combat.

Whenever a grenade, landmine or other explosive detonates, all players within a 30m radius will be flagged as being engaged in combat (can be increased/decreased/disabled in config).

You can also trigger the combat flag when a player fires a gun, regardless of whether it was at another player or not (disabled by default).

You can also trigger the combat flag when a bullet impact is detected near a player (disabled by default).

You can make the mod spawn a road flare on a player if they exit the game before their logout timer finishes to reveal their character's location (disabled by default).

You can kill a player for exiting the game before their logout timer finishes (disabled by default).

I've designed the mod to limit its impact on server performance, there shouldn't be any issues with the default config even on high pop servers. 

If you turn the 'TriggerOnBulletImpactRadius' setting on, there may be a hit to performance on servers with lots of gunfire as every time a bullet impacts a surface the server will scan for nearby players to flag as being in combat. This feature is experimental and it's disabled by default. It shouldn't really be needed, the default method for detecting gunshots at players should suffice in 99% of circumstances.

The mod may potentially be less accurate at detecting gunshots fired at players more than 1km away especially if the player raises their rifle to compensate for bullet drop, but in my testing it seems to work most of the time at long distances and I had no issues detecting close-quarters gunfights.

Should work for all guns including modded rifles, but let me know if you encounter any bugs or issues.

Default Behavior:

So with the default config, the mod behaves as follows:
If a player shoots a gun at another player (or multiple players), regardless of whether the bullet deals damage to the other player, all players involved will be flagged as being in combat.
If a player shoots a gun while standing within ~5m of another player(s), all players will be flagged as being in combat (regardless of whether they are friends/squadmates or not).
If a player throws a grenade that explodes (or a chem gas grenade), all players within a 30m radius of the explosion will be flagged as being in combat.

Once a player is flagged as being in combat, the server saves a timestamp of when the combat began, and a countdown begins (5 minutes by default). If the player attempts to logout (or Alt F4's, crashes or disconnects from the server) then their character will stay on the server for the duration of the timer.

When the player logs out before their combat log timer expires, a message will be displayed on the logout window explaining why their timer is extended and the potential penalty they will suffer (no penalties by default, but this can be adjusted in the JSON config).

This gives any players engaged in combat plenty of time to find the player and kill them, resulting in less admin requests/server reports from your players.

The messages are stringtable'd and translated to all languages, but if the language translation is wrong for your language feel free to repack the mod and change the stringtable.csv file.

Default Configuration:

CombatLogoutSecs = 300; // How many seconds to keep player on server after engaging in combat
DisableExitButtonSecs = 5; // How many seconds to disable the Exit button when logging out after combat (set to same as CombatLogoutSecs to disable button entirely)
NotifyPlayerOfPenalty = true; // Whether or not to notify the player of the penalty for combat logging on the logout window text
TriggerOnGunshot = false; // Whether or not to trigger the combat log timer for simply firing your gun (hunting, raiding etc)
TriggerOnBulletImpactRadius = 0; // // Experimental (set to 0 to disable) - Radius in meters around bullet impact to detect and flag players for engaging in combat
TriggerOnExplosiveRadius = 30; // Radius around explosions to detect and flag players for engaging in combat (includes ChemGas grenades, set to 0 to disable)
DropFlareOnPlayer = 0; // Whether or not to drop a flare on a player (0 = disabled, 1 = when they logout before timer, 2 = when they are auto-killed for combat logging)
KillPlayer = 0; // Whether or not to kill a player for leaving the server before the combat log timer runs out

Installation Instructions:

Install this mod like any other mod - copy it into your server folder and add it to your mods list. Make sure to copy the .bikey into your server keys if you're not using a server management tool like OmegaManager which does that automatically.

When you run the mod for the first time a default JSON config will be created in your server profile: %server_root%/profiles/Zenarchist/ZenAntiCombatLogout.json

This JSON file allows you to modify various settings, such as the logout timer, 

This mod must be installed on both the server & client.

Repack & Source Code:

You can repack this mod if you like, and do anything else you want with it for that matter. The source code is on my GitHub at www.zenarchist.io

Enjoy!