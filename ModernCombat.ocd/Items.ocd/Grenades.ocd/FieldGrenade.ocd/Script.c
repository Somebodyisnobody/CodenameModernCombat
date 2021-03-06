#include CMC_Library_GrenadeItem

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";

local ContactIncinerate = 1;
local NoBurnDecay = 1;
local Collectible = true;

local Grenade_ContainedDamage = 120;
local Grenade_SmokeColor = 0xffdd0000;

/* --- Callbacks --- */

// What happens when the grenade explodes
public func OnDetonation()
{
	Explosion([30, 60], [110, 80], true);
}
