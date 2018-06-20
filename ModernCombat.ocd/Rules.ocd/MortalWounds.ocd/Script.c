/**
	Prevent instant death.
	
	@author Marky
*/

#include Library_Singleton

/* --- Properties --- */

local Name = "$Name$";
local Description = "$Description$";
local Visibility = VIS_Editor;
local EditorPlacementLimit = 1; // Rules are to be placed only once

/* --- Engine Callbacks --- */

// Message window if rule is selected in the player menu
func Activate(int for_player)
{
	MessageWindow(this.Description, for_player);
	return true;
}

func Construction(object creator)
{
	_inherited(creator, ...);
	
	SetDelayedDeath(false); // Creating the rule disables delayed death
}

func Destruction()
{
	SetDelayedDeath(true); // Destroying it resotres delayed death for everyone
}

/* --- Interface --- */


// When crew is being recruited - non recruited crew may die normally :D
public func SetDelayedDeath(bool enable, object crew)
{
	if (nil == crew) // Set for all?
	{
		for (var crew in FindObjects(Find_OCF(OCF_CrewMember), Find_Func("IsClonk")))
		{
			if (crew) SetDelayedDeath(enable, crew);
		}
	}
	else // specific
	{
		var check = GetEffect("RuleMortalWoundsCheck", crew);
		if (!check)
		{
			crew->CreateEffect(RuleMortalWoundsCheck, 1, 1); // Does not need to be top priority, let other effects modify damage first, and so on
		}
	}
}

public func GetDelayedDeathEffect(object target)
{
	return GetEffect("RuleMortalWoundsCheck", target);
}

/* --- Functionality --- */

local RuleMortalWoundsCheck = new Effect
{
	Name = "RuleMortalWoundsCheck",
	TimerMax = 15 * 35,
	
	Start = func (int temp)
	{
		if (!temp)
		{
			this.Target.IsIncapacitated = CMC_Rule_MortalWounds.IsIncapacitated;
			this.Target.DoReanimate = CMC_Rule_MortalWounds.DoReanimate;
			this.Target.GetAlive = CMC_Rule_MortalWounds.GetAlive;
			this.Target.GetEnergy = CMC_Rule_MortalWounds.GetEnergy;
			this.Target.GetOCF = CMC_Rule_MortalWounds.GetOCF;
			this.is_incapacitated = false;
			this.death_timer = this.TimerMax;
		}
		return FX_OK;
	},
	
	Timer = func ()
	{
		var change = +1;
		// Already wounded?
		if (IsIncapacitated())
		{
			if (this.death_timer <= 1) // Would reduce to 0 at the end of the function, but skip another timer call
			{
				this.Target->Kill(); // TODO: This will very likely cause problems with kill tracing at the moment
				return FX_Execute_Kill;
			}
			change = -1;
		}
		
		this.death_timer = BoundBy(this.death_timer + change, 0, this.TimerMax);
	},
	
	Damage = func (int health_change, int cause, int by_player)
	{
		// Already wounded?
		if (IsIncapacitated())
		{
			// Do not die if the change would kill you
			if (health_change < 0)
			{
				return 0;
			}
			else // Healing is allowed
			{
				return health_change;
			}
		}
		// In case the victim would die
		else if (health_change < 0 && (health_change + 1000 * this.Target->GetEnergy() <= 0))
		{
			var stay_at_one_health = (1 - this.Target->GetEnergy()) * 1000;
			this.is_incapacitated = true;
			this.Target->~OnIncapacitated(health_change, cause, by_player);
			return stay_at_one_health;
		}
		return health_change;
	},
	
	// Status
	
	IsIncapacitated = func ()
	{
		return this.is_incapacitated;
	},
	
	DoReanimate = func (int by_player)
	{
		if (IsIncapacitated())
		{	
			this.is_incapacitated = false;
			this.Target->~OnReanimated(by_player);
			return true;
		}
		return false;
	},
	
};

/* --- Functions, are being added to clonks if the rule is active --- */

func IsIncapacitated()
{
	var fx = GetEffect("RuleMortalWoundsCheck", this);
	return fx && fx->IsIncapacitated();
}

func DoReanimate(int by_player)
{
	var fx = GetEffect("RuleMortalWoundsCheck", this);
	return fx && fx->DoReanimate(by_player);
}

func GetAlive()
{
	if (this->~IsIncapacitated())
	{
		return false;
	}
	else
	{
		// Calling inherited calls the inherited function of the rule, instead of the object.
		// Using the ID is a reference seems fair enough, most objects do not overload
		// these functions anyway.
		return Call(this->GetID().GetAlive); 
	}
}

func GetOCF()
{
	// Calling inherited calls the inherited function of the rule, instead of the object.
	var ocf = Call(this->GetID().GetOCF);

	// Remove alive flag if incapacitated	
	if (this->~IsIncapacitated() && (ocf & OCF_Alive))
	{
		return ocf - OCF_Alive;
	}
	else
	{
		return ocf;
	}
}

func GetEnergy()
{	
	if (this->~IsIncapacitated())
	{
		return 0;
	}
	else
	{
		// Calling inherited calls the inherited function of the rule, instead of the object.
		return Call(this->GetID().GetEnergy);
	}
}
