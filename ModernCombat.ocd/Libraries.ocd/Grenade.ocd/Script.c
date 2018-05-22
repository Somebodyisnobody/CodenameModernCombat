/**
	Library for grenades

	@author Marky
*/

#include Library_RangedWeapon

/* --- Properties --- */

local grenade_detonated = false; // bool: already detonated? Important for preventing multiple explosions, etc.
local grenade_active = false;    // bool: active?
local grenade_aiming = false;    // bool: aiming? Used for separating "ironsight" toggle from normal clonk aiming

local animation_set;

local Grenade_ContainedDamage = 60;
local Grenade_FuseTime = 105; // 3 seconds
local Grenade_ThrowSpeed = 20; // Multiplication factor to clonk.ThrowSpeed
local Grenade_ThrowDelay = 20; // Time between consecutive throws
local Grenade_MaxDamage = 10; // Take this many damage and it activates itself

local DefaultShootTime = 16;
local DefaultShootTime2 = 8;

func NoDecoDamage(){ return true; } // Forgot what this does, but lets leave it in there for now
func IsBouncy(){ return true; } // Gets launched by jump pad

/* --- Using the grenade --- */

// Use holding controls?
public func HoldingEnabled()
{
	return true;
}


public func RejectUse(object user)
{
	return !user->HasActionProcedure(false); // The clonk must be able to use the hands
}


public func ControlUseStart(object user, int x, int y)
{
	if (IsGrenadeHoldEnabled(user->GetController()))
	{
		Fuse();
	}
	return true;
}

public func ControlUseHolding(object user, int x, int y)
{
	if (IsGrenadeHoldEnabled())
	{
		user->SetAimPosition(GetAimPosition(user, x, y), true);
	}
	return true;
}

public func ControlUseCancel(object user, int x, int y)
{
	if (IsGrenadeHoldEnabled(user->GetController()) && grenade_active)
	{
		DoDrop(user);
	}
	return true;
}

public func ControlUseStop(object user, int x, int y)
{
	if (IsGrenadeHoldEnabled(user->GetController()))
	{
		LaunchGrenade(user, x, y);
	}
	else
	{
		if (grenade_active)
		{
			LaunchGrenade(user, x, y);
		}
		else
		{
			Fuse();
		}
	}
	return true;
}


public func ControlUseAltStart(object user, int x, int y)
{
	ToggleAim(user, x, y);
	return true;
}


public func ControlUseAltCancel(object user, int x, int y)
{
	ToggleAim(user, x, y, true); // Make sure that it is toggled off
	return true;
}


/* --- Aiming --- */


// Right click will by default be a toggle control but can be switched to a holding control
public func IsGrenadeHoldEnabled(int player)
{
	return CMC_Player_Settings->GetConfigurationValue(player, CMC_GRENADE_HOLD, true);
}


// Called by the CMC modified clonk, see ModernCombat.ocd\System.ocg\Mod_Clonk.c
public func ControlUseAiming(object user, int x, int y)
{
	if (IsGrenadeHoldEnabled())
	{
		// In this case, the control should never fire
		SetAimingCursor(user, false, true);
		return true;
	}

	user->SetAimPosition(GetAimPosition(user, x, y), true);
	
	if (!user->HasActionProcedure(false))
	{
		CancelAim(user);
	}
}

func GetAimPosition(object user, int x, int y)
{
	// Save new angle
	var angle = Angle(0, 0, x, y);
	angle = Normalize(angle, -180);

	if (angle >  160) angle =  160;
	if (angle < -160) angle = -160;

	return angle;
}


func ToggleAim(object user, int x, int y, bool ensure_off)
{
	if (grenade_aiming)
	{
		grenade_aiming = false;
		StopAim(user);
		RemoveAimAnimation(user);
	}
	else if (!ensure_off)
	{
		StartAim(user, GetAimPosition(user, x, y));
		grenade_aiming = true;
	}
}


func StartAim(object user, int angle)
{
	if (!user->IsAiming())
	{
		user->StartAim(this, angle);
	}
	SetAimingCursor(user, true);
}


func StopAim(object user)
{
	if (user->IsAiming())
	{
		user->StopAim();
	}
	SetAimingCursor(user, false);
}


func CancelAim(object user)
{
	user->CancelAiming(this);
	if (grenade_active)
	{
		DoDrop(user);
	}
	ToggleAim(user, nil, nil, true);
}


func LaunchGrenade(object user, int x, int y)
{
	if (grenade_aiming)
	{
		ThrowAimed(user, GetAimPosition(user, x, y));
	}
	else
	{
		StartLob(user, GetAimPosition(user, x, y));
	}
}


func ThrowAimed(object user, int angle)
{
	if (GetEffect("BlockGrenadeThrow", user))
	{
		return false;
	}
	
	user->StopAim();
	return true;
}


func SetAimingCursor(object user, bool value, bool forced)
{
	var controller = user->GetController();
	if (forced || !IsGrenadeHoldEnabled(controller))
	{
		// Mouse move will adjust aim angle
		SetPlayerControlEnabled(controller, CON_CMC_AimingCursor, value);
		// Disable OC default
		SetPlayerControlEnabled(controller, CON_Aim, !value);
	}
}

func RemoveAimAnimation(object user)
{
	user->StopAnimation(user->GetRootAnimation(CLONK_ANIM_SLOT_Arms));
}


/* --- Engine callbacks --- */

func Initialize()
{
	DefaultLoadTime = this.Grenade_ThrowDelay;
	animation_set = {
		AimMode         = AIM_Position, // The aiming animation is done by adjusting the animation position to fit the angle
		AnimationAim    = "SpearAimArms",
		AnimationShoot  = "SpearThrowArms",
		AnimationShoot2 = "SpearThrow2Arms",
		AnimationShoot3 = "SpearThrow3Arms",
		WalkBack        =  56,
	};
	_inherited(...);
}


func Hit(int xdir, int ydir)
{
	PlaySoundHit();
	if (GBackSolid( 0, +5)) return SetYDir(-ydir/26);
	if (GBackSolid( 0, -5)) return SetYDir(-ydir/26);
	if (GBackSolid(-5,  0)) return SetXDir(-xdir/16);
	if (GBackSolid(+5,  0)) return SetXDir(-xdir/16);
}


func Collection(object by_object)
{
 	if (by_object->GetCategory() & C4D_Living)
 	{
    	PlaySoundDeploy();
	}
	_inherited(by_object, ...);
}


func Departure(object from)
{
	if (grenade_active)
	{
		from->PlayerMessage(from->GetController(), "");
	}
	grenade_aiming = false;
	_inherited(from, ...);
}


func Damage(int change, int cause, int cause_plr)
{
	// Only do stuff if the object has the HitPoints property.
	if (this && this.Grenade_MaxDamage != nil)
	{
		if (GetDamage() >= this.Grenade_MaxDamage)
		{
			Fuse();
		}
	}
	return _inherited(change, cause, cause_plr, ...);
}

/* --- Callbacks from loading/aiming system --- */

func GetAnimationSet() { return animation_set; }


// Callback from the clonk, when he actually has stopped aiming
func FinishedAiming(object user, int angle)
{
	if (grenade_aiming)
	{
		user->StartShoot(this);
		grenade_aiming = false;
		StopAim(user);
	}
	return true;
}


// Called in the half of the shoot animation (when ShootTime2 is over)
public func DuringShoot(object user, int angle)
{
	DoThrow(user, angle);
}


/* --- Sounds --- */


func PlaySoundActivate()
{
	Sound("Grenade::Activate");
}


func PlaySoundDeploy()
{
	Sound("Grenade::Deploy?");
}


func PlaySoundDetonation()
{
	Sound("Grenade::Explosion?");
}


func PlaySoundHit()
{
	Sound("Grenade::Hit?", {multiple = true});
}


func PlaySoundThrow()
{
	Sound("Grenade::Throw?", {multiple = true});
}


/* --- Overloadable callbacks --- */

// What happens when the grenade explodes
public func OnDetonation()
{
	Explode(30, true, 60);
}

// How the trail is drawn
public func HandleTrail()
{
	if (!Contained())
	{
		var speed = Abs(GetXDir()) + Abs(GetYDir());
		var alpha = Min(205, 105 + speed);
		var lifetime_base = speed / 3;
		
		var color = SplitRGBaValue(this.Grenade_SmokeColor ?? RGB(100, 100, 100));
		
		CreateParticle("Smoke",
		               -GetXDir() / 6, -GetYDir() / 6,
		                PV_Random(-10, 10), -5,
		                PV_Random(10, 20),
		               {
			               Prototype = Particles_Thrust(),
			               Size = PV_Linear((lifetime_base + 20) / 10, (lifetime_base + 60) / 10),
			               R = color.R, G = color.G, B = color.B, Alpha = alpha
		               });
	}
}


/* --- Explosion logic --- */

// Callback that triggers detonation.
func Detonate() 
{
	RemoveEffect("GrenadeFuse", this, nil, true);

	if (!grenade_detonated)
	{
		grenade_detonated = true;
		PlaySoundDetonation();
		this->OnDetonation();
	}
}

// Callback for giving the container damage.
func DetonateInContainer()
{
	var container = Contained();
	if (container && this.Grenade_ContainedDamage > 0)
	{
		if (container->GetAlive())
		{
			if (!container->Contained())
			{
				container->Fling(container->GetXDir()/10, container->GetYDir()/10 - 1);
			}
			container->DoEnergy(-this.Grenade_ContainedDamage, false, FX_Call_EngBlast, this->GetController()); // FIXME: may be replaced with custom damage system
		}
	}
	Exit();
}

/* --- Countdown until explosion --- */

func Fuse()
{
	if (!grenade_active)
	{
		PlaySoundActivate();
		grenade_active = true;
		grenade_detonated = false;
  		SetGraphics("Active");
		CreateEffect(GrenadeFuse, 200, 1);
		this.Collectible = false;
		SetCategory(C4D_Vehicle);
	}
}

local GrenadeFuse = new Effect
{
	Timer = func (int time)
	{
		if (time > this.Target.Grenade_FuseTime)
		{
			this.Target->DetonateInContainer();
			this.Target->Detonate();
			return FX_Execute_Kill;
		}

		var container = this.Target->Contained();
		if (container)
		{
			if (container->~IsClonk() && !container->GetAlive() || container->~IsIncapacitated()) // FIXME: callback for wounded clonk
			{
      			this.Target->DoDrop(container);
      		}
			else if ((container.GetHandItem && this.Target == container->GetHandItem(0)) // Has inventory control? => Get correct item
			     || (!container.GetHandItem && this.Target == container->Contents(0)))   // No inventory control? => First item
			{
			    var progress = BoundBy(time * 1000 / Max(1, this.Target.Grenade_FuseTime), 0, 1000);
				var color = InterpolateRGBa(progress,   0, RGB(0, 255, 0),
				                                      500, RGB(255, 255, 0),
				                                     1000, RGB(255, 0, 0));

				container->PlayerMessage(container->GetController(),"<c %x>{{Rock}}</c>", color); // FIXME: placeholder for graphics
			}
		}
		else
		{
			this.Target->~HandleTrail();
		}

		return FX_OK;
	},
};

/* --- Animations --- */

func Launch(object user)
{
	user = user ?? Contained();
	Exit();
	if (user)
	{
		SetController(user->GetController());
		RemoveEffect("BlockGrenadeThrow", user);
		AddEffect("BlockGrenadeThrow", user, 1, this.Grenade_ThrowDelay, user);
		user->UpdateAttach();
	}
	SetRDir(RandomX(-6, +6));
}

func StartLob(object user, int angle)
{
	if (Normalize(angle, -180) >= 0)
		user->SetDir(DIR_Right);
	else
		user->SetDir(DIR_Left);

	if (user->~IsClonk())
	{
		user->~SetHandAction(1); // Set hands ocupied
		user->CreateEffect(ThrowingAnimation, 1, 1, this, angle);
	}
}

func FinishLob(object user, int angle)
{
	Launch(user);
	var div = 60;
	SetVelocity(angle, 30);
	AddSpeed(div * user->GetXDir(1000) / 100, 
	         div * user->GetYDir(1000) / 100 - 50,
	         1000);
	SetPosition(user->GetX(), user->GetY() + 2);
	if (!GetEffect("RollingFriction", this))
	{
		CreateEffect(RollingFriction, 1, 70); // Roll with reduced friction for 2 seconds
	}
	SetRDir(Sign(GetXDir()) * 10);
 }

func DoThrow(object user, int angle)
{
	var div = 60; // 40% is converted to the direction of the throwing angle.
	var xdir = user->GetXDir(1000);
	var ydir = user->GetYDir(1000);
	var speed = user.ThrowSpeed * Grenade_ThrowSpeed + (100 - div) * Distance(xdir, ydir) / 100;
	var grenade_x = div * xdir / 100 + Sin(angle, speed);
	var grenade_y = div * ydir / 100 - Cos(angle, speed);

	Launch(user);
	SetXDir(grenade_x, 1000);
	SetYDir(grenade_y, 1000);
	SetPosition(user->GetX(), user->GetY() - 6);
}

func DoDrop(object user)
{
	Launch(user);
	if (user)
	{
		SetSpeed(user->GetXDir(), user->GetYDir());
	}
}

local ThrowingAnimation = new Effect
{
	Start = func (int temporary, object thrown, int angle)
	{
		if (!temporary)
		{
			this.Throw_Time = 16;
			this.Throw_Object = thrown;
			this.Throw_Angle = angle;
			this.Target->PlayAnimation("ThrowArms", CLONK_ANIM_SLOT_Arms, Anim_Linear(0, 0, this.Target->GetAnimationLength("ThrowArms"), this.Throw_Time));
		}
	},

	Timer = func(int time)
	{
		if (this.Throw_Object)
		{
			if (time == this.Throw_Time * 8 / 15)
			{
				this.Throw_Object->FinishLob(this.Target, this.Throw_Angle);
			}
			if (time < this.Throw_Time)
			{
				return FX_OK;
			}
		}
		return FX_Execute_Kill;
	},

	Stop = func (int temporary)
	{
		if(!temporary)
		{
			this.Target->StopAnimation(this.Target->GetRootAnimation(CLONK_ANIM_SLOT_Arms));
			this.Target->~SetHandAction(0);
		}
	},
};


local RollingFriction = new Effect
{
	Start = func (int temporary)
	{
		if (!temporary)
		{
			this.Friction = [];
			for (var i = 0; i < this.Target->GetVertexNum(); ++i)
			{
				this.Friction[i] = this.Target->GetVertex(i, VTX_Friction);
				this.Target->SetVertex(i, VTX_Friction, this.Friction[i] / 4, 2);
			}
		}
	},
	
	Stop = func (int temporary)
	{
		if (!temporary)
		{
			for (var i = 0; i < this.Target->GetVertexNum(); ++i)
			{
				this.Target->SetVertex(i, VTX_Friction, this.Target->GetID()->GetVertex(i, VTX_Friction), 2);
			}
		}
	},
};