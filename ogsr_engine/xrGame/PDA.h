#pragma once

#include "..\xr_3da\feel_touch.h"
#include "hud_item_object.h"

#include "InfoPortionDefs.h"
#include "character_info_defs.h"

#include "PdaMsg.h"
#include "hudsound.h"
#include "ai_sounds.h"

class CInventoryOwner;
class CPda;

DEF_VECTOR (PDA_LIST, CPda*);

class CPda :
	public CHudItemObject,
	public Feel::Touch
{
	typedef	CHudItemObject inherited;
	
	HUD_SOUND		sndShow;
	HUD_SOUND		sndHide;
	
	ESoundTypes		m_eSoundShow;
	ESoundTypes		m_eSoundHide;
public:
											CPda					(ESoundTypes eSoundType = SOUND_TYPE_WEAPON_SUBMACHINEGUN);
	virtual									~CPda					();

	virtual BOOL 							net_Spawn				(CSE_Abstract* DC);
	virtual void 							Load					(LPCSTR section);
	virtual void 							net_Destroy				();
	virtual	void net_Relcase( CObject* O );

	virtual void 							OnH_A_Chield			();
	virtual void 							OnH_B_Independent		(bool just_before_destroy);

	virtual void 							shedule_Update			(u32 dt);

	virtual void 							feel_touch_new			(CObject* O);
	virtual void 							feel_touch_delete		(CObject* O);
	virtual BOOL 							feel_touch_contact		(CObject* O);

	virtual void			Hide( bool = false );
	virtual void			Show( bool = false );
	virtual void			UpdateHudAdditonal		(Fmatrix& trans);
	
public:
	virtual void OnStateSwitch(u32 S, u32 oldState);
	virtual void OnAnimationEnd(u32 state);
	virtual void OnMoveToRuck(EItemPlace prevPlace);
	virtual void UpdateCL();
	virtual void UpdateXForm();
	virtual void OnActiveItem();
	virtual void OnHiddenItem();

public:
	virtual u16								GetOriginalOwnerID		() {return m_idOriginalOwner;}
	virtual CInventoryOwner*				GetOriginalOwner		();
	virtual CObject*						GetOwnerObject			();


			void							TurnOn					();
			void							TurnOff					();
	
			bool 							IsActive				() {return IsOn();}
			bool 							IsOn					() {return !m_bTurnedOff;}
			bool 							IsOff					() {return m_bTurnedOff;}


			xr_map<u16, CPda*>				ActivePDAContacts		();
			CPda*							GetPdaFromOwner			(CObject* owner);
			u32								ActiveContactsNum		()							{return m_active_contacts.size();}


	virtual void							save					(NET_Packet &output_packet);
	virtual void							load					(IReader &input_packet);

	virtual LPCSTR							Name					();

protected:
	void									UpdateActiveContacts	();


	xr_vector<CObject*>						m_active_contacts;
	float									m_fRadius;
        bool m_changed;

	u16										m_idOriginalOwner;
	shared_str					m_SpecificChracterOwner;
	xr_string								m_sFullName;

	bool									m_bTurnedOff;
};
