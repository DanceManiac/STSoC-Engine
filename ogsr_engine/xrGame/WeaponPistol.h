#pragma once
#include "weaponcustompistol.h"

class CWeaponPistol :
	public CWeaponCustomPistol
{
	typedef CWeaponCustomPistol inherited;
public:
	CWeaponPistol	(LPCSTR name);
	virtual ~CWeaponPistol(void);

	virtual void	Load			(LPCSTR section);
	
	virtual void	switch2_Reload	();
	virtual void	switch2unmiss	();

	//virtual void	OnShot			();
	virtual void	OnStateSwitch	(u32 S, u32 oldstate) override;
	virtual void	OnAnimationEnd	(u32 state) override;
	virtual void	net_Destroy		();
	virtual void	OnH_B_Chield	();

	virtual void	OnDrawUI();
	virtual void	net_Relcase(CObject *object);

	//анимации
	virtual void	PlayAnimShow	() override;
	//virtual void	PlayAnimBore	() override;
	virtual void	PlayAnimIdleSprint() override;
	virtual void	PlayAnimIdleMoving() override;
	virtual void	PlayAnimIdleMovingCrouch() override;
	virtual void	PlayAnimIdleMovingSlow() override;
	virtual void	PlayAnimIdleMovingCrouchSlow() override;
	virtual void	PlayAnimIdle	() override;
	virtual void	PlayAnimAim		() override;
	virtual void	PlayAnimHide	() override;
	virtual void	PlayAnimReload	() override;
	virtual void	PlayAnimShoot	() override;

	virtual void	UpdateSounds	();
	BOOL					CheckForMiss		();
protected:	
	bool bMiss;
	virtual bool	AllowFireWhileWorking() {return true;}

	HUD_SOUND			sndClose;
	ESoundTypes			m_eSoundClose;

	bool m_opened;
};
