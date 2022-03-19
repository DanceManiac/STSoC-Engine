#include "stdafx.h"
#include "bolt.h"
#include "ParticlesObject.h"
#include "PhysicsShell.h"
#include "xr_level_controller.h"
#include "ai_sounds.h"
#include "actor.h"

CBolt::CBolt(void) 
{
	m_weight					= .1f;
	SetSlot( BOLT_SLOT );
	m_flags.set					(Fruck, FALSE);
	m_thrower_id				=u16(-1);
}

void CBolt::Load(LPCSTR section) {
	inherited::Load(section);
	HUD_SOUND::LoadSound(section, "snd_throw", m_ThrowSnd, SOUND_TYPE_WEAPON_SHOOTING);
}

CBolt::~CBolt(void) 
{
}

void CBolt::OnH_A_Chield() 
{
	inherited::OnH_A_Chield();
	CObject* o= H_Parent()->H_Parent();
	if(o)SetInitiator(o->ID());
	//HUD_SOUND::DestroySound(m_ThrowSnd);
}

void CBolt::OnEvent(NET_Packet& P, u16 type) 
{
	inherited::OnEvent(P,type);
}

bool CBolt::Activate( bool now )
{
	Show( now );
	return true;
}

void CBolt::Deactivate( bool now )
{
	Hide( now || ( GetState() == eThrowStart || GetState() == eReady || GetState() == eThrow ) );
}

void CBolt::Throw() 
{
	if(Actor())
		PlaySound(m_ThrowSnd, Actor()->Position());
	CMissile					*l_pBolt = smart_cast<CMissile*>(m_fake_missile);
	if(!l_pBolt)				return;
	l_pBolt->set_destroy_time	(u32(m_dwDestroyTimeMax/phTimefactor));
	inherited::Throw			();
	spawn_fake_missile			();
}

bool CBolt::Useful() const
{
	return false;
}

bool CBolt::Action(s32 cmd, u32 flags) 
{
	return inherited::Action(cmd, flags);
}

void CBolt::Destroy()
{
	inherited::Destroy();
}

void CBolt::activate_physic_shell()
{
	inherited::activate_physic_shell();
	m_pPhysicsShell->SetAirResistance(.0001f);
}

void CBolt::SetInitiator(u16 id)
{
	m_thrower_id = id;
}

u16	CBolt::Initiator()
{
	return m_thrower_id;
}

void CBolt::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count)
{
	str_name = NameShort();
	str_count = "";
	icon_sect_name = *cNameSect();
}