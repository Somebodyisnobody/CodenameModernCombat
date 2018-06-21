#appendto Clonk

/* --- Properties --- */

local ActMap = {
Incapacitated = {
	Prototype = Action,
	Name = "Incapacitated",
	Directions = 2,
	Length = 1,
	Delay = 0,
	NextAction = "Hold",
	ObjectDisabled = 1,
},
};

/* --- Engine callbacks --- */

func Recruitment(int player)
{
	// If the rule object does not exist (rule object disables the delayed death)
	CMC_Rule_MortalWounds->SetDelayedDeath(!CMC_Rule_MortalWounds->GetInstance(), this);
	return _inherited(player, ...);
}

/* --- Incapacitation --- */

func OnIncapacitated(int health_change, int cause, int by_player)
{
	SetAction("Incapacitated");
	StartDeathAnimation(CLONK_ANIM_SLOT_Death - 1);
}

func OnReanimated(int by_player)
{
	// Remove animations
	var fx = CMC_Rule_MortalWounds->GetDelayedDeathEffect(this);
	if (fx) for (var anim in fx.animations)
	{
		StopAnimation(anim);
	}
	// Get up!
	this->~DoKneel();
}

/* --- Better death animation --- */

func StartDead()
{
	StartDeathAnimation();
}

func StartDeathAnimation(int animation_slot)
{
	animation_slot = animation_slot ?? CLONK_ANIM_SLOT_Death;
	
	// Save animation slots
	var fx = CMC_Rule_MortalWounds->GetDelayedDeathEffect(this);
	if (fx) fx.animations = [];
	
	// Blend death animation with other animations, except for the death slot
	var merged_animations = false;	
	for (var slot = 0; slot < animation_slot; ++slot)
	{
		if (GetRootAnimation(slot) == nil) continue;
		var anim = OverlayDeathAnimation(slot);
		merged_animations = true;
		
		if (fx)
		{
			PushBack(fx.animations, anim);
		}
	}

	// Force the death animation if there were no other animations active
	if (!merged_animations)
	{
		var anim = OverlayDeathAnimation(animation_slot);
		if (fx)
		{
			PushBack(fx.animations, anim);
		}
	}

	// Update carried items
	UpdateAttach();
	// Set proper turn type
	SetTurnType(1);
}

// Merges the animation in an animation slot with the death animation
// More variation is possible if we considered adding a random value for the length,
// or if we set the parameters according to current speed, etc.
func OverlayDeathAnimation(int slot, string animation)
{
	animation = animation ?? "Dead";
	return PlayAnimation(animation, slot, Anim_Linear(0, 0, GetAnimationLength(animation), 20, ANIM_Hold), Anim_Linear(0, 0, 1000, 10, ANIM_Remove));
}