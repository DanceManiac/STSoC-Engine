#include "stdafx.h"
#include "weaponpistol.h"
#include "ParticlesObject.h"
#include "actor.h"

CWeaponPistol::CWeaponPistol(LPCSTR name) : CWeaponCustomPistol(name)
{
	m_eSoundClose		= ESoundTypes(SOUND_TYPE_WEAPON_RECHARGING /*| eSoundType*/);
	m_opened = false;
	SetPending(FALSE);
}

CWeaponPistol::~CWeaponPistol(void)
{
}

void CWeaponPistol::net_Destroy()
{
	inherited::net_Destroy();

	// sounds
	HUD_SOUND::DestroySound(sndClose);
}

void CWeaponPistol::net_Relcase(CObject *object)
{
	inherited::net_Relcase(object);
}

void CWeaponPistol::OnDrawUI()
{
	inherited::OnDrawUI();
}

void CWeaponPistol::Load	(LPCSTR section)
{
	inherited::Load		(section);

	HUD_SOUND::LoadSound(section, "snd_close", sndClose, m_eSoundClose);
}

void CWeaponPistol::OnH_B_Chield		()
{
	inherited::OnH_B_Chield		();
	m_opened = false;
}

void CWeaponPistol::PlayAnimShow()
{
	VERIFY(GetState()==eShowing);
	if (iAmmoElapsed >= 1)
		m_opened = false;
	else
		m_opened = true;
		
	if (m_opened)
		PlayHUDMotion("anm_show_empty", FALSE, this, GetState());
	else
		inherited::PlayAnimShow();
}

void CWeaponPistol::PlayAnimBore()
{
    if (iAmmoElapsed == 0)
        PlayHUDMotion("anm_bore_empty", "anim_empty", TRUE, this, GetState());
    else
        inherited::PlayAnimBore();
}

void CWeaponPistol::PlayAnimIdleSprint()
{
	if(GetState()!=eIdle)
		return;
	if (m_opened)
	{
		if(!AnmSprintStartPlayed && AnimationExist("anm_idle_sprint_start_empty"))
		{
			PlayHUDMotion("anm_idle_sprint_start_empty", false, nullptr, GetState());
			AnmSprintStartPlayed = true;
			return;
		}
		else if (AnimationExist("anm_idle_sprint_empty") || AnimationExist("anm_idle_sprint"))
			PlayHUDMotion("anm_idle_sprint_empty", "anm_idle_sprint", false, nullptr, GetState());
	}
	else
		inherited::PlayAnimIdleSprint();
}

bool CWeaponPistol::PlayAnimIdleSprintEnd()
{
	if (m_opened && AnimationExist(std::string("anm_idle_sprint_end_empty").c_str())) {
		PlayHUDMotion(std::string("anm_idle_sprint_end_empty").c_str(), false, nullptr, GetState());
		return true;
	}
	else
		return inherited::PlayAnimIdleSprintEnd();
}

void CWeaponPistol::PlayAnimIdleMoving()
{
	if(GetState()!=eIdle)
		return;
	if (m_opened)
		PlayHUDMotion("anm_idle_moving_empty", TRUE, nullptr, GetState());
	else
		inherited::PlayAnimIdleMoving();
}

void CWeaponPistol::PlayAnimIdleMovingCrouch()
{
	if (m_opened)
		PlayHUDMotion("anm_idle_moving_crouch_empty", "anm_empty", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleMovingCrouch();
}

void CWeaponPistol::PlayAnimIdleMovingSlow()
{
	if (m_opened)
		PlayHUDMotion("anm_idle_moving_slow_empty", "anm_idle_moving_empty", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleMovingSlow();
}

void CWeaponPistol::PlayAnimIdleMovingCrouchSlow()
{
	if (m_opened)
		PlayHUDMotion("anm_idle_moving_crouch_slow_empty", AnimationExist("anm_idle_moving_crouch_empty") ? "anm_idle_moving_crouch_empty" : "anm_idle_moving_empty", true, nullptr, GetState());
	else
		inherited::PlayAnimIdleMovingCrouchSlow();
}

void CWeaponPistol::PlayAnimIdle()
{
	VERIFY(GetState()==eIdle);
	if(GetState()!=eIdle)
		return;

	if (TryPlayAnimIdle()) 
		return;

	if (IsZoomed())
		PlayAnimAim();
	else if (m_opened)
	{
		if (IsRotatingFromZoom())
		{
			if (AnimationExist("anm_idle_aim_end_empty")) {
				PlayHUDMotion("anm_idle_aim_end_empty", true, nullptr, GetState());
				return;
			}
		}
		PlayHUDMotion("anm_idle_empty", TRUE, nullptr, GetState());
	}
	else
		inherited::PlayAnimIdle();
}

void CWeaponPistol::PlayAnimAim()
{
	if (m_opened)
	{
		if (IsRotatingToZoom())
		{
			if (AnimationExist("anm_idle_aim_start_empty"))
			{
				PlayHUDMotion("anm_idle_aim_start_empty", true, nullptr, GetState());
				return;
			}
		}

		if (const char* guns_aim_anm = GetAnimAimName())
		{
			string64 guns_aim_anm_full;
			xr_strconcat(guns_aim_anm_full, guns_aim_anm, "_empty");
			if (AnimationExist(guns_aim_anm_full))
			{
				PlayHUDMotion(guns_aim_anm_full, true, nullptr, GetState());
				return;
			}
		}
		PlayHUDMotion("anm_idle_aim_empty", TRUE, nullptr, GetState());
	}
	else
		inherited::PlayAnimAim();
}

void CWeaponPistol::PlayAnimReload()
{
	VERIFY(GetState() == eReload);
	if (bMisfire && AnimationExist("anm_misfire"))
	{
		PlayHUDMotion("anm_misfire", TRUE, nullptr, GetState());
	}
	else if (m_opened)
		PlayHUDMotion("anm_reload_empty", TRUE, nullptr, GetState());
	else 
		inherited::PlayAnimReload();
	m_opened = false;
}

void CWeaponPistol::PlayAnimHide()
{
	VERIFY(GetState() == eHiding);
	if (m_opened)
	{
		PlaySound(sndClose, get_LastFP());
		PlayHUDMotion("anm_hide_empty", TRUE, this, GetState());
	}
	else
		inherited::PlayAnimHide();
}

void CWeaponPistol::PlayAnimShoot()
{
	VERIFY(GetState() == eFire || GetState() == eFire2);
	string_path guns_shoot_anm{};
	xr_strconcat(guns_shoot_anm, "anm_shots", (this->IsZoomed() && !this->IsRotatingToZoom()) ? "_aim" : "", iAmmoElapsed == 1 ? "_last" : "", this->IsSilencerAttached() ? "_sil" : "");
	if (AnimationExist(guns_shoot_anm)) {
		Msg("--[%s] Play anim [%s] for [%s]", __FUNCTION__, guns_shoot_anm, this->cNameSect().c_str());
		PlayHUDMotion(guns_shoot_anm, false, this, GetState());
		return;
	}
	else
		Msg("!![%s] anim [%s] not found for [%s]", __FUNCTION__, guns_shoot_anm, this->cNameSect().c_str());


	if (iAmmoElapsed > 1)
	{
		PlayHUDMotion("anim_shoot", "anm_shots", FALSE, this, GetState());
		m_opened = false;
	}
	else
	{
		PlayHUDMotion("anim_shot_last", "anm_shot_l", FALSE, this, GetState());
		m_opened = true;
	}
}

void CWeaponPistol::switch2_Reload()
{
//.	if(GetState()==eReload) return;
	inherited::switch2_Reload();
}

void CWeaponPistol::switch2unmiss()
{
	//PlaySound	(sndShow,get_LastFP());

	SetPending(TRUE);
	PlayHUDMotion("anm_shots_lightmisfire", /*TRUE*/FALSE, nullptr, GetState());
	//PlayAnimShow();
}

#include "HUDManager.h"
void CWeaponPistol::OnStateSwitch(u32 S, u32 oldState)
{
	switch (S)
	{
	case eMiss:
		/*if(smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity()==H_Parent()) )
		{
			HUD().GetUI()->AddInfoMessage("gun_miss");
			// Callbacks added by Cribbledirge.
			//StateSwitchCallback(GameObject::eOnActorWeaponJammed, GameObject::eOnNPCWeaponJammed);
			switch2unmiss();
		}*/
		break;
	}
	inherited::OnStateSwitch(S, oldState);
}

void CWeaponPistol::OnAnimationEnd(u32 state)
{
	if(state == eHiding && m_opened) 
	{
		m_opened = false;
//		switch2_Hiding();
	} 
	switch(state)
	{
	case eMiss:
		{
			SetPending(FALSE);
			SwitchState(eIdle);
			break;
		}
	}
	inherited::OnAnimationEnd(state);
}

/*
void CWeaponPistol::OnShot		()
{
	// Sound
	PlaySound		(*m_pSndShotCurrent,get_LastFP());

	AddShotEffector	();
	
	PlayAnimShoot	();

	// Shell Drop
	Fvector vel; 
	PHGetLinearVell(vel);
	OnShellDrop					(get_LastSP(),  vel);

	// Огонь из ствола
	
	StartFlameParticles	();
	R_ASSERT2(!m_pFlameParticles || !m_pFlameParticles->IsLooped(),
			  "can't set looped particles system for shoting with pistol");
	
	//дым из ствола
	StartSmokeParticles	(get_LastFP(), vel);
}
*/

void CWeaponPistol::UpdateSounds()
{
	inherited::UpdateSounds();

	if (sndClose.playing())
		sndClose.set_position(get_LastFP());
}

BOOL CWeaponPistol::CheckForMiss	()
{
	if (OnClient()) return FALSE;

	if ( Core.Features.test( xrCore::Feature::npc_simplified_shooting ) ) {
	  CActor *actor = smart_cast<CActor*>( H_Parent() );
	  if ( !actor ) return FALSE;
	}
	
	float random = ::Random.randF(0.f,1.f);
	float rnd = ::Random.randF(0.f,1.f);
	float mp = GetConditionMisfireProbability();
	if(AnimationExist("anm_shots_lightmisfire") && random > 0.6f && rnd < mp && iAmmoElapsed > 1)
	{
		FireEnd();
		
		bMiss = true;
		SwitchState(eMiss);	
		if(smart_cast<CActor*>(this->H_Parent()) && (Level().CurrentViewEntity()==H_Parent()) )
		{
			HUD().GetUI()->AddInfoMessage("gun_miss");
			// Callbacks added by Cribbledirge.
			//StateSwitchCallback(GameObject::eOnActorWeaponJammed, GameObject::eOnNPCWeaponJammed);
			switch2unmiss();
		}		
		
		return TRUE;
	}
	else
	{
		return CWeapon::CheckForMisfire();
	}
}