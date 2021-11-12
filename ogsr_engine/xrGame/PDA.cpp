#include "stdafx.h"
#include "pda.h"
#include "hudmanager.h"
#include "PhysicsShell.h"
#include "Entity.h"
#include "actor.h"

#include "xrserver.h"
#include "xrServer_Objects_ALife_Items.h"
#include "level.h"

#include "specific_character.h"
#include "alife_registry_wrappers.h"
#include "UIGameSP.h"
#include "ui/UIPDAWnd.h"

#include "player_hud.h"
#include "UIGameCustom.h"
#include "ui\UIPdaWnd.h"
#include "ai_sounds.h"
#include "Inventory.h"
#include "../xr_3da/IGame_Persistent.h"

CPda::CPda(void)						
{										
	SetSlot( PDA_SLOT );
	m_flags.set				(Fruck, TRUE);

	m_idOriginalOwner		= u16(-1);
	m_SpecificChracterOwner = nullptr;

	TurnOff					();

	m_bZoomed = false;
	m_eDeferredEnable = eDefault;
	joystick = BI_NONE;
	target_screen_switch = 0.f;
	m_fLR_CameraFactor = 0.f;
	m_fLR_MovingFactor = 0.f;
	m_fLR_InertiaFactor = 0.f;
	m_fUD_InertiaFactor = 0.f;
	m_bNoticedEmptyBattery = false;
}

CPda::~CPda() 
{}

BOOL CPda::net_Spawn(CSE_Abstract* DC) 
{
	inherited::net_Spawn		(DC);
	CSE_Abstract				*abstract = (CSE_Abstract*)(DC);
	CSE_ALifeItemPDA			*pda = smart_cast<CSE_ALifeItemPDA*>(abstract);
	R_ASSERT					(pda);
	m_idOriginalOwner			= pda->m_original_owner;
	m_SpecificChracterOwner		= pda->m_specific_character;

	return						(TRUE);
}

void CPda::net_Destroy() 
{
	inherited::net_Destroy		();
	TurnOff						();
	feel_touch.clear			();
}

#include "HudSound.h"
extern CUIWindow* GetPdaWindow();
void CPda::Load(LPCSTR section) 
{
	inherited::Load(section);

	m_fRadius = pSettings->r_float(section,"radius");
	m_fDisplayBrightnessPowerSaving = READ_IF_EXISTS(pSettings, r_float, section, "power_saving_brightness", .6f);
	m_fPowerSavingCharge = READ_IF_EXISTS(pSettings, r_float, section, "power_saving_charge", .15f);
	m_joystick_bone = READ_IF_EXISTS(pSettings, r_string, section, "joystick_bone", nullptr);
	/*HUD_SOUND::LoadSound(section, "snd_draw", "sndShow", true);
	HUD_SOUND::LoadSound(section, "snd_holster", "sndHide", true);
	HUD_SOUND::LoadSound(section, "snd_draw_empty", "sndShowEmpty", true);
	HUD_SOUND::LoadSound(section, "snd_holster_empty", "sndHideEmpty", true);
	HUD_SOUND::LoadSound(section, "snd_btn_press", "sndButtonPress");
	HUD_SOUND::LoadSound(section, "snd_btn_release", "sndButtonRelease");
	HUD_SOUND::LoadSound(section, "snd_empty", "sndEmptyBattery", true);*/
	m_screen_on_delay = READ_IF_EXISTS(pSettings, r_float, section, "screen_on_delay", 0.f);
	m_screen_off_delay = READ_IF_EXISTS(pSettings, r_float, section, "screen_off_delay", 0.f);
	m_thumb_rot[0] = READ_IF_EXISTS(pSettings, r_float, section, "thumb_rot_x", 0.f);
	m_thumb_rot[1] = READ_IF_EXISTS(pSettings, r_float, section, "thumb_rot_y", 0.f);
}

void CPda::shedule_Update(u32 dt)	
{
	inherited::shedule_Update	(dt);

	if(!H_Parent()) return;
	Position().set	(H_Parent()->Position());

	if( IsOn() && Level().CurrentEntity() && Level().CurrentEntity()->ID()==H_Parent()->ID() )
	{
		CEntityAlive* EA = smart_cast<CEntityAlive*>(H_Parent());
		if(!EA || !EA->g_Alive())
		{
			TurnOff();
			return;
		}

                m_changed = false;
		feel_touch_update(Position(),m_fRadius);
		UpdateActiveContacts	();

                if ( m_changed ) {
                  if ( HUD().GetUI() ) {
                    CUIGameSP* pGameSP = smart_cast<CUIGameSP*>( HUD().GetUI()->UIGame() );
                    if ( pGameSP )
                      pGameSP->PdaMenu->PdaContentsChanged( pda_section::contacts );
                  }
                  m_changed = false;
                }
	}
}

void CPda::UpdateActiveContacts	()
{
	if ( !m_changed ) {
          for ( auto& it : m_active_contacts ) {
            CEntityAlive* pEA = smart_cast<CEntityAlive*>( it );
            if ( !pEA->g_Alive() ) {
              m_changed = true;
              break;
            }
          }
	}

	m_active_contacts.clear_not_free();
	xr_vector<CObject*>::iterator it= feel_touch.begin();
	for(;it!=feel_touch.end();++it){
		CEntityAlive* pEA = smart_cast<CEntityAlive*>(*it);
		if(!!pEA->g_Alive())
			m_active_contacts.push_back(*it);
	}
}

void CPda::feel_touch_new(CObject* O) 
{
	CInventoryOwner* pNewContactInvOwner	= smart_cast<CInventoryOwner*>(O);
	CInventoryOwner* pOwner					= smart_cast<CInventoryOwner*>( H_Parent() );VERIFY(pOwner);

	pOwner->NewPdaContact					(pNewContactInvOwner);
        m_changed = true;
}

void CPda::feel_touch_delete(CObject* O) 
{
	if(!H_Parent())							return;
	CInventoryOwner* pLostContactInvOwner	= smart_cast<CInventoryOwner*>(O);
	CInventoryOwner* pOwner					= smart_cast<CInventoryOwner*>( H_Parent() );VERIFY(pOwner);

	pOwner->LostPdaContact					(pLostContactInvOwner);
        m_changed = true;
}

BOOL CPda::feel_touch_contact(CObject* O) 
{
	CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>(O);
	if(pInvOwner){
		if( this!=pInvOwner->GetPDA() )
		{
			CEntityAlive* pEntityAlive = smart_cast<CEntityAlive*>(O);
			if(pEntityAlive && !pEntityAlive->cast_base_monster() )
				return TRUE;
		}else
		return FALSE;
	}

	return FALSE;
}

void CPda::OnH_A_Chield() 
{
	VERIFY(IsOff());
	//включить PDA только если оно находится у первого владельца
	if(H_Parent()->ID() == m_idOriginalOwner){
		TurnOn					();
		if(m_sFullName.empty()){
			m_sFullName.assign(inherited::Name());
			m_sFullName += " ";
			m_sFullName += (smart_cast<CInventoryOwner*>(H_Parent()))->Name();
		}
	};
	inherited::OnH_A_Chield		();
}

void CPda::OnH_B_Independent(bool just_before_destroy) 
{
	inherited::OnH_B_Independent(just_before_destroy);
	
	//выключить
	TurnOff();

	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (!pActor) return;

	//m_sounds.PlaySound(hasEnoughBatteryPower() ? "sndHide" : "sndHideEmpty", Position(), H_Root(), !!GetHUDmode(), false);

	SwitchState(eHidden);
	SetPending(FALSE);
	m_bZoomed = false;
	m_fZoomfactor = 0.f;

	CUIPdaWnd* pda = (CUIPdaWnd*)GetPdaWindow();
	//if (pda->IsShown()) pda->HideDialog();
	g_player_hud->reset_thumb(true);
	pda->ResetJoystick(true);

	if (joystick != BI_NONE && HudItemData())
		HudItemData()->m_model->LL_GetBoneInstance(joystick).reset_callback();

	g_player_hud->detach_item(this);
}


CInventoryOwner* CPda::GetOriginalOwner()
{
	CObject* pObject =  Level().Objects.net_Find(GetOriginalOwnerID());
	CInventoryOwner* pInvOwner = smart_cast<CInventoryOwner*>(pObject);

	return pInvOwner;
}



xr_map<u16, CPda*> CPda::ActivePDAContacts()
{
	xr_map<u16, CPda*> res;

	for (auto* Obj : m_active_contacts)
		if (CPda* p = GetPdaFromOwner(Obj))
			res.emplace( Obj->ID(), p );

	return res;
}

void CPda::save(NET_Packet &output_packet)
{
	inherited::save	(output_packet);
	save_data		(m_sFullName, output_packet);
}

void CPda::load(IReader &input_packet)
{
	inherited::load	(input_packet);
	load_data		(m_sFullName, input_packet);
}

CObject* CPda::GetOwnerObject()
{
	return				Level().Objects.net_Find(GetOriginalOwnerID());
}
LPCSTR		CPda::Name				()
{
	if( !m_SpecificChracterOwner.size() )
		return inherited::Name();

	if(m_sFullName.empty())
	{
		m_sFullName.assign(inherited::Name());
		
		CSpecificCharacter spec_char;
		spec_char.Load(m_SpecificChracterOwner);
		m_sFullName += " ";
		m_sFullName += xr_string(spec_char.Name());
	}
	
	return m_sFullName.c_str();
}

CPda* CPda::GetPdaFromOwner(CObject* owner)
{
	return smart_cast<CInventoryOwner*>(owner)->GetPDA			();
}


void CPda::TurnOn() {
  m_bTurnedOff = false;
  m_changed    = true;
}


void CPda::TurnOff() {
  m_bTurnedOff = true;
  m_active_contacts.clear();
}


void CPda::net_Relcase( CObject *O ) {
  inherited::net_Relcase( O );
  if ( m_active_contacts.size() && !Level().is_removing_objects() ) {
      const auto I = std::find( m_active_contacts.begin(), m_active_contacts.end(), O );
      if ( I != m_active_contacts.end() )
        m_active_contacts.erase( I );
  }
}

void CPda::OnStateSwitch(u32 S, u32 oldState)
{
	inherited::OnStateSwitch(S, oldState);

	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (!pActor) return;

	switch (S)
	{
	case eShowing:
	{
		g_player_hud->attach_item(this);
		g_pGamePersistent->pda_shader_data.pda_display_factor = 0.f;

		//m_sounds.PlaySound(hasEnoughBatteryPower() ? "sndShow" : "sndShowEmpty", Position(), H_Root(), !!GetHUDmode(), false);
		PlayHUDMotion(!m_bNoticedEmptyBattery ? "anm_show" : "anm_show_empty", FALSE, this, GetState());
		
		SetPending(TRUE);
		target_screen_switch = Device.fTimeGlobal + m_screen_on_delay;
	}
	break;
	case eHiding:
	{
		if (oldState != eHiding)
		{
			//m_sounds.PlaySound(hasEnoughBatteryPower() ? "sndHide" : "sndHideEmpty", Position(), H_Root(), !!GetHUDmode(), false);
			PlayHUDMotion(!m_bNoticedEmptyBattery ? "anm_hide" : "anm_hide_empty", TRUE, this, GetState());
			SetPending(TRUE);
			m_bZoomed = false;
			((CUIPdaWnd*)GetPdaWindow())->Enable(false);
			g_player_hud->reset_thumb(false);
			((CUIPdaWnd*)GetPdaWindow())->ResetJoystick(false);
			if (joystick != BI_NONE && HudItemData())
				HudItemData()->m_model->LL_GetBoneInstance(joystick).reset_callback();
			target_screen_switch = Device.fTimeGlobal + m_screen_off_delay;
		}
	}
	break;
	case eHidden:
	{
		if (oldState != eHidden)
		{
			m_bZoomed = false;
			m_fZoomfactor = 0.f;
			CUIPdaWnd* pda = (CUIPdaWnd*)GetPdaWindow();

			if (pda->IsShown())
			{
				pda->Enable(true);
			}

			g_player_hud->reset_thumb(true);
			pda->ResetJoystick(true);
		}
		SetPending(FALSE);
	}
	break;
	case eIdle:
	{
		PlayAnimIdle();

		if (m_joystick_bone && joystick == BI_NONE && HudItemData())
			joystick = HudItemData()->m_model->LL_BoneID(m_joystick_bone);

		if (joystick != BI_NONE && HudItemData())
		{
			CBoneInstance* bi = &HudItemData()->m_model->LL_GetBoneInstance(joystick);
			if (bi)
				bi->set_callback(bctCustom, JoystickCallback, this);
		}
	}
	break;
	case eEmptyBattery:
	{
		SetPending(TRUE);
		//m_sounds.PlaySound("sndEmptyBattery", Position(), H_Root(), !!GetHUDmode(), false);
		PlayHUDMotion("anm_empty", TRUE, this, GetState());
		m_bNoticedEmptyBattery = true;
	}
	}
}

void CPda::OnAnimationEnd(u32 state)
{
	inherited::OnAnimationEnd(state);
	switch (state)
	{
	case eShowing:
	{
		if (!hasEnoughBatteryPower() && !m_bNoticedEmptyBattery)
		{
			SwitchState(eEmptyBattery);
			return;
		}
		SetPending(FALSE);
		SwitchState(eIdle);
	}
	break;
	case eHiding:
	{
		SetPending(FALSE);
		SwitchState(eHidden);
		g_player_hud->detach_item(this);
	}
	break;
	case eEmptyBattery:
	{
		SetPending(FALSE);
		SwitchState(eIdle);
	}
	break;
	}
}

void CPda::JoystickCallback(CBoneInstance* B)
{
	//CPda* Pda = static_cast<CPda*>(B->callback_param());
	CUIPdaWnd* pda = (CUIPdaWnd*)GetPdaWindow();

	static float fAvgTimeDelta = Device.fTimeDelta;
	fAvgTimeDelta = _inertion(fAvgTimeDelta, Device.fTimeDelta, 0.8f);

	Fvector& target = pda->target_joystickrot;
	Fvector& current = pda->joystickrot;
	float& target_press = pda->target_buttonpress;
	float& press = pda->buttonpress;

	if (!target.similar(current, .0001f))
	{
		Fvector diff;
		diff = target;
		diff.sub(current);
		diff.mul(fAvgTimeDelta / .1f);
		current.add(diff);
	}
	else
		current.set(target);

	if (!fsimilar(target_press, press, .0001f))
	{
		//float prev_press = press;

		float diff = target_press;
		diff -= press;
		diff *= (fAvgTimeDelta / .1f);
		press += diff;

		/*if (prev_press == 0.f && press < 0.f)
			Pda->m_sounds.PlaySound("sndButtonPress", B->mTransform.c, Pda->H_Root(), !!Pda->GetHUDmode());
		else if (prev_press < -.001f && press >= -.001f)
			Pda->m_sounds.PlaySound("sndButtonRelease", B->mTransform.c, Pda->H_Root(), !!Pda->GetHUDmode());*/
	}
	else
		press = target_press;

	Fmatrix rotation;
	rotation.identity();
	rotation.rotateX(current.x);

	Fmatrix rotation_y;
	rotation_y.identity();
	rotation_y.rotateY(current.y);
	rotation.mulA_43(rotation_y);

	rotation_y.identity();
	rotation_y.rotateZ(current.z);
	rotation.mulA_43(rotation_y);

	rotation.translate_over(0.f, press, 0.f);

	B->mTransform.mulB_43(rotation);
}

extern bool IsMainMenuActive();

void CPda::UpdateCL()
{
	inherited::UpdateCL();

	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (!pActor) return;

	// For battery icon
	float condition = GetCondition();
	CUIPdaWnd* pda = (CUIPdaWnd*)GetPdaWindow();
	pda->m_power = condition;

	u32 state = GetState();
	bool enoughBatteryPower = hasEnoughBatteryPower();

	if (pda->IsShown())
	{
		// Hide PDA UI on low condition (battery) or when the item is hidden.
		if (!enoughBatteryPower || state == eHidden)
		{
			//pda->HideDialog();
			m_bZoomed = false;

			if (state == eIdle)
				SwitchState(eEmptyBattery);
		}
		else
		{
			// Force update PDA UI if it's disabled (no input) and check for deferred enable or zoom in.
			if (!pda->IsEnabled())
			{
				pda->Update();
				if (m_eDeferredEnable == eEnable || m_eDeferredEnable == eEnableZoomed)
				{
					/*if (Actor()->cam_freelook == eflDisabled && g_player_hud->script_anim_part == u8(-1))
					{*/
						pda->Enable(true);
						m_bZoomed = m_eDeferredEnable == eEnableZoomed;
						m_eDeferredEnable = eDefault;
					//}
				}
			}

			// Disable PDA UI input if player is sprinting and no deferred input enable is expected.
			else
			{
				CEntity::SEntityState st;
				Actor()->g_State(st);
				if (st.bSprint && !st.bCrouch && !m_eDeferredEnable)
				{
					pda->Enable(false);
					m_bZoomed = false;
				}
			}

			// Turn on "power saving" on low battery charge (dims the screen).
			if (/*IsUsingCondition() && */condition < m_fPowerSavingCharge)
			{
				if (!m_bPowerSaving)
				{
					luabind::functor<void> funct;
					if (ai().script_engine().functor("pda.on_low_battery", funct))
						funct();
					m_bPowerSaving = true;
				}
			}

			// Turn off "power saving" if battery has sufficient charge.
			else if (m_bPowerSaving)
				m_bPowerSaving = false;
		}
	}
	else
	{
		// Show PDA UI if possible
		if (!IsMainMenuActive() && state != eHiding && state != eHidden && enoughBatteryPower)
		{
			//pda->ShowDialog(false); // Don't hide indicators
			m_bNoticedEmptyBattery = false;
			if (m_eDeferredEnable == eEnable) // Don't disable input if it was enabled before opening the Main Menu.
				m_eDeferredEnable = eDefault;
			else
				pda->Enable(false);
		}
	}

	if (GetState() != eHidden)
	{
		// Adjust screen brightness (smooth)
		if (m_bPowerSaving)
		{
			if (g_pGamePersistent->pda_shader_data.pda_displaybrightness > m_fDisplayBrightnessPowerSaving)
				g_pGamePersistent->pda_shader_data.pda_displaybrightness -= Device.fTimeDelta / .25f;
		}
		else
			g_pGamePersistent->pda_shader_data.pda_displaybrightness = 1.f;

		clamp(g_pGamePersistent->pda_shader_data.pda_displaybrightness, m_fDisplayBrightnessPowerSaving, 1.f);

		// Screen "Glitch" factor
		g_pGamePersistent->pda_shader_data.pda_psy_influence = m_psy_factor;

		// Update Display Visibility (turn on/off)
		if (target_screen_switch < Device.fTimeGlobal)
		{
			if (!enoughBatteryPower || state == eHiding)
				// Change screen transparency (towards 0 = not visible).
				g_pGamePersistent->pda_shader_data.pda_display_factor -= Device.fTimeDelta / .25f;
			else
				// Change screen transparency (towards 1 = fully visible).
				g_pGamePersistent->pda_shader_data.pda_display_factor += Device.fTimeDelta / .75f;
		}

		clamp(g_pGamePersistent->pda_shader_data.pda_display_factor, 0.f, 1.f);
	}
}

void CPda::OnMoveToRuck()
{
	inherited::OnMoveToRuck();

	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (!pActor) return;

	SwitchState(eHidden);
	if (joystick != BI_NONE && HudItemData())
		HudItemData()->m_model->LL_GetBoneInstance(joystick).reset_callback();
	g_player_hud->detach_item(this);

	//CUIPdaWnd* pda = &CurrentGameUI()->GetPdaMenu();
	//if (pda->IsShown()) pda->HideDialog();
	StopCurrentAnimWithoutCallback();
	SetPending(FALSE);
}

void CPda::UpdateHudAdditional(struct _matrix<float> & pidor)
{
}

void CPda::UpdateHudAdditonal(Fmatrix& trans)
{
	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (!pActor) return;

	u8 idx = GetCurrentHudOffsetIdx();

	float m_fZoomRotationFactor = 0.f, m_fZoomRotateTime = 0.f;
	//============= Поворот ствола во время аима =============//
	if ((pActor->IsZoomAimingMode() && m_fZoomRotationFactor <= 1.f) ||
		(!pActor->IsZoomAimingMode() && m_fZoomRotationFactor > 0.f))
	{
		attachable_hud_item* hi = HudItemData();
		R_ASSERT(hi);
		Fvector curr_offs, curr_rot;
		curr_offs = hi->m_measures.m_hands_offset[hud_item_measures::m_hands_offset_pos][idx];
		curr_rot = hi->m_measures.m_hands_offset[hud_item_measures::m_hands_offset_rot][idx];
		curr_offs.mul(m_fZoomRotationFactor);
		curr_rot.mul(m_fZoomRotationFactor);

		Fmatrix hud_rotation;
		hud_rotation.identity();
		hud_rotation.rotateX(curr_rot.x);

		Fmatrix hud_rotation_y;
		hud_rotation_y.identity();
		hud_rotation_y.rotateY(curr_rot.y);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation_y.identity();
		hud_rotation_y.rotateZ(curr_rot.z);
		hud_rotation.mulA_43(hud_rotation_y);

		hud_rotation.translate_over(curr_offs);
		trans.mulB_43(hud_rotation);

		if (pActor->IsZoomAimingMode())
		{
			m_fZoomRotationFactor += Device.fTimeDelta / m_fZoomRotateTime;
		}
		else
		{
			m_fZoomRotationFactor -= Device.fTimeDelta / m_fZoomRotateTime;
		}
		clamp(m_fZoomRotationFactor, 0.f, 1.f);
	}
	clamp(idx, 0ui8, 1ui8);

	/*static float fAvgTimeDelta = Device.fTimeDelta;
	attachable_hud_item* hi = HudItemData();

	//============= Сдвиг оружия при стрельбе =============//
	// Параметры сдвига
	float fShootingReturnSpeedMod = _lerp(
		hi->m_measures.m_shooting_params.m_ret_speed,
		hi->m_measures.m_shooting_params.m_ret_speed_aim,
		m_fZoomRotationFactor);

	float fShootingBackwOffset = _lerp(
		hi->m_measures.m_shooting_params.m_shot_offset_BACKW.x,
		hi->m_measures.m_shooting_params.m_shot_offset_BACKW.y,
		m_fZoomRotationFactor);

	Fvector4 vShOffsets; // x = L, y = R, z = U, w = D
	vShOffsets.x = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD.x,
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD_aim.x,
		m_fZoomRotationFactor);
	vShOffsets.y = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD.y,
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD_aim.y,
		m_fZoomRotationFactor);
	vShOffsets.z = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD.z,
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD_aim.z,
		m_fZoomRotationFactor);
	vShOffsets.w = _lerp(
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD.w,
		hi->m_measures.m_shooting_params.m_shot_max_offset_LRUD_aim.w,
		m_fZoomRotationFactor);

	// Плавное затухание сдвига от стрельбы (основное, но без линейной никогда не опустит до полного 0.0f)
	m_fLR_ShootingFactor *= clampr(1.f - fAvgTimeDelta * fShootingReturnSpeedMod, 0.0f, 1.0f);
	m_fUD_ShootingFactor *= clampr(1.f - fAvgTimeDelta * fShootingReturnSpeedMod, 0.0f, 1.0f);
	m_fBACKW_ShootingFactor *= clampr(1.f - fAvgTimeDelta * fShootingReturnSpeedMod, 0.0f, 1.0f);

	// Минимальное линейное затухание сдвига от стрельбы при покое (горизонталь)
	{
		float fRetSpeedMod = fShootingReturnSpeedMod * 0.125f;
		if (m_fLR_ShootingFactor < 0.0f)
		{
			m_fLR_ShootingFactor += fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fLR_ShootingFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fLR_ShootingFactor -= fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fLR_ShootingFactor, 0.0f, 1.0f);
		}
	}

	// Минимальное линейное затухание сдвига от стрельбы при покое (вертикаль)
	{
		float fRetSpeedMod = fShootingReturnSpeedMod * 0.125f;
		if (m_fUD_ShootingFactor < 0.0f)
		{
			m_fUD_ShootingFactor += fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fUD_ShootingFactor, -1.0f, 0.0f);
		}
		else
		{
			m_fUD_ShootingFactor -= fAvgTimeDelta * fRetSpeedMod;
			clamp(m_fUD_ShootingFactor, 0.0f, 1.0f);
		}
	}

	// Минимальное линейное затухание сдвига от стрельбы при покое (вперёд\назад)
	{
		float fRetSpeedMod = fShootingReturnSpeedMod * 0.125f;
		m_fBACKW_ShootingFactor -= fAvgTimeDelta * fRetSpeedMod;
		clamp(m_fBACKW_ShootingFactor, 0.0f, 1.0f);
	}

	// Применяем сдвиг от стрельбы к худу
	{
		float fLR_lim = (m_fLR_ShootingFactor < 0.0f ? vShOffsets.x : vShOffsets.y);
		float fUD_lim = (m_fUD_ShootingFactor < 0.0f ? vShOffsets.z : vShOffsets.w);

		Fvector curr_offs;
		curr_offs = { fLR_lim * m_fLR_ShootingFactor, fUD_lim * -1.f * m_fUD_ShootingFactor, -1.f * fShootingBackwOffset * m_fBACKW_ShootingFactor };

		Fmatrix hud_rotation;
		hud_rotation.identity();
		hud_rotation.translate_over(curr_offs);
		trans.mulB_43(hud_rotation);
	}*/
}


void CPda::Hide(bool now)
{
	if (now)
	{
		OnStateSwitch(eHidden, GetState());
		SetState(eHidden);
		StopHUDSounds();
	}
	else
		SwitchState(eHiding);

	m_bZoomed = false;
}

void CPda::Show(bool now)
{
	if (now)
	{
		StopCurrentAnimWithoutCallback();
		OnStateSwitch(eIdle, GetState());
		SetState(eIdle);
		StopHUDSounds();
	}
	else
		SwitchState(eShowing);
}

void CPda::UpdateXForm()
{
	CInventoryItem::UpdateXForm();
}

void CPda::OnActiveItem()
{
	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (!pActor) return;

	SwitchState(eShowing);
}

void CPda::OnHiddenItem()
{
	auto pActor = smart_cast<const CActor*>(H_Parent());
	if (!pActor) return;

	SwitchState(eHiding);
}