#include "stdafx.h"
#include "weaponBM16.h"

CWeaponBM16::~CWeaponBM16()
{
	HUD_SOUND::DestroySound(m_sndReload1);
}

void CWeaponBM16::Load	(LPCSTR section)
{
	inherited::Load		(section);

	HUD_SOUND::LoadSound(section, "snd_reload_1", m_sndReload1, m_eSoundReload);
}

void CWeaponBM16::PlayReloadSound()
{
	if (bMisfire && AnimationExist("anm_misfire"))
		PlaySound	(sndMisfire,get_LastFP());
	else if ( m_magazine.size() == 1 || !HaveCartridgeInInventory( 2 ) )
		PlaySound	(m_sndReload1,get_LastFP());
	else
		PlaySound	(sndReload,get_LastFP());
}

void CWeaponBM16::UpdateSounds()
{
	inherited::UpdateSounds();

	if (m_sndReload1.playing())	m_sndReload1.set_position(get_LastFP());
}

void CWeaponBM16::PlayAnimShoot()
{
	if (m_magazine.empty())
		return;

	string_path guns_shoot_anm{};
	xr_strconcat(guns_shoot_anm, "anm_shots", (this->IsZoomed() && !this->IsRotatingToZoom()) ? "_aim_" : "_", std::to_string(m_magazine.size()).c_str());
	if (AnimationExist(guns_shoot_anm)) {
		Msg("--[%s] Play anim [%s] for [%s]", __FUNCTION__, guns_shoot_anm, this->cNameSect().c_str());
		PlayHUDMotion(guns_shoot_anm, false, this, GetState());
		return;
	}
	else
		Msg("!![%s] anim [%s] not found for [%s]", __FUNCTION__, guns_shoot_anm, this->cNameSect().c_str());

	switch (m_magazine.size())
	{
	case 1: PlayHUDMotion("anm_shot_1", FALSE, this, GetState()); break;
	case 2: PlayHUDMotion("anm_shot_2", FALSE, this, GetState()); break;
	default: PlayHUDMotion("anm_shots", FALSE, this, GetState()); break;
	}
}

void CWeaponBM16::PlayAnimShow()
{
	VERIFY(GetState() == eShowing);

	switch (m_magazine.size())
	{
	case 0:
	{
			PlayHUDMotion("anm_show_0", TRUE, this, GetState());
	} break;
	case 1:
	{
			PlayHUDMotion("anm_show_1", TRUE, this, GetState());
	} break;
	case 2:
	{
			PlayHUDMotion("anm_show_2", TRUE, this, GetState());
	} break;
	}
}

void CWeaponBM16::PlayAnimHide()
{
	VERIFY(GetState() == eHiding);

	switch (m_magazine.size())
	{
	case 0:
	{
			PlayHUDMotion("anm_hide_0", TRUE, this, GetState());
	} break;
	case 1:
	{
			PlayHUDMotion("anm_hide_1", TRUE, this, GetState());
	} break;
	case 2:
	{
			PlayHUDMotion("anm_hide_2", TRUE, this, GetState());
	} break;
	}
}

void CWeaponBM16::PlayAnimReload()
{
	VERIFY(GetState() == eReload);
	if (bMisfire && AnimationExist("anm_misfire"))
	{
		bMisfire = false;
		PlayHUDMotion("anm_misfire", TRUE, nullptr, GetState());
	}
	else if (m_magazine.size() == 1 || !HaveCartridgeInInventory(2))
		PlayHUDMotion("anm_reload_1", TRUE, this, GetState());
	else
		PlayHUDMotion("anm_reload_2", TRUE, this, GetState());
}

void CWeaponBM16::PlayAnimIdleMoving()
{
	switch (m_magazine.size())
	{
	case 0:
	{
			PlayHUDMotion("anm_idle_moving_0", TRUE, this, GetState());
	} break;
	case 1:
	{
			PlayHUDMotion("anm_idle_moving_1", TRUE, this, GetState());
	} break;
	case 2:
	{
			PlayHUDMotion("anm_idle_moving_2", TRUE, this, GetState());
	} break;
	}
}

void CWeaponBM16::PlayAnimIdleMovingCrouch()
{
	switch (m_magazine.size())
	{
	case 0: PlayHUDMotion("anm_idle_moving_crouch_0", "anm_idle_moving_0", true, nullptr, GetState()); break;
	case 1: PlayHUDMotion("anm_idle_moving_crouch_1", "anm_idle_moving_1", true, nullptr, GetState()); break;
	case 2: PlayHUDMotion("anm_idle_moving_crouch_2", "anm_idle_moving_2", true, nullptr, GetState()); break;
	}
}

void CWeaponBM16::PlayAnimIdleMovingSlow()
{
	switch (m_magazine.size())
	{
	case 0: PlayHUDMotion("anm_idle_moving_slow_0", "anm_idle_moving_0", true, nullptr, GetState()); break;
	case 1: PlayHUDMotion("anm_idle_moving_slow_1", "anm_idle_moving_1", true, nullptr, GetState()); break;
	case 2: PlayHUDMotion("anm_idle_moving_slow_2", "anm_idle_moving_2", true, nullptr, GetState()); break;
	}
}

void CWeaponBM16::PlayAnimIdleMovingCrouchSlow()
{
	switch (m_magazine.size())
	{
	case 0: PlayHUDMotion("anm_idle_moving_crouch_slow_0", AnimationExist("anm_idle_moving_crouch_0") ? "anm_idle_moving_crouch_0" : "anm_idle_moving_0", true, nullptr, GetState()); break;
	case 1: PlayHUDMotion("anm_idle_moving_crouch_slow_1", AnimationExist("anm_idle_moving_crouch_1") ? "anm_idle_moving_crouch_1" : "anm_idle_moving_1", true, nullptr, GetState()); break;
	case 2: PlayHUDMotion("anm_idle_moving_crouch_slow_2", AnimationExist("anm_idle_moving_crouch_2") ? "anm_idle_moving_crouch_2" : "anm_idle_moving_2", true, nullptr, GetState()); break;
	}
}

void CWeaponBM16::PlayAnimIdleSprint()
{
	switch (m_magazine.size())
	{
	case 0:
	{
			PlayHUDMotion("anm_idle_sprint_0", TRUE, this, GetState());
	} break;
	case 1:
	{
			PlayHUDMotion("anm_idle_sprint_1", TRUE, this, GetState());
	} break;
	case 2:
	{
			PlayHUDMotion("anm_idle_sprint_2", TRUE, this, GetState());
	} break;
	}
}

void CWeaponBM16::PlayAnimIdle()
{
	if (TryPlayAnimIdle())
		return;

	if (IsZoomed())
	{
		switch (m_magazine.size())
		{
		case 0:
		{ 
			PlayHUDMotion("anm_idle_aim_0", TRUE, nullptr, GetState());
		}
		break;
		case 1: 
		{
			PlayHUDMotion("anm_idle_aim_1", TRUE, nullptr, GetState());
		}
		break;
		case 2:
		{
				PlayHUDMotion("anm_idle_aim_2", TRUE, nullptr, GetState());
		}
		break;
		};
	}
	else
	{
		switch (m_magazine.size())
		{
		case 0: 
		{
			PlayHUDMotion("anm_idle_0", TRUE, nullptr, GetState());
		}
		break;
		case 1: 
		{ 
			PlayHUDMotion("anm_idle_1", TRUE, nullptr, GetState());
		}
		break;
		case 2: 
		{ 
			PlayHUDMotion("anm_idle_2", TRUE, nullptr, GetState());
		}
		break;
		};
	}
}
