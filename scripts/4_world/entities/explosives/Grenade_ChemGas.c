modded class Grenade_ChemGas
{
	// Chem grenade explodes on contact and doesn't trigger OnExplode()
	override void EOnContact(IEntity other, Contact extra)
	{
		super.EOnContact(other, extra);

		#ifdef SERVER
		TriggerCombatPlayersInRadius(GetZenAntiCombatLogoutConfig().TriggerOnExplosiveRadius);
		#endif
	}
};
