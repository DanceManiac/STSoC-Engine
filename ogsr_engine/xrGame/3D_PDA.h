#include "WeaponMagazined.h"
#include "PDA.h"
#include "ui/uipdawnd.h"
#include "UIDialogHolder.h"

class C3DPDA : public CWeaponMagazined, public CUIPdaWnd, public CDialogHolder
{
	typedef CWeaponMagazined inherited;
	typedef CUIPdaWnd PDA;
public:
	C3DPDA() {}
	virtual ~C3DPDA() {}


	virtual void OnStateSwitch(u32 S, u32 oldState) override
	{
		inherited::OnStateSwitch(S, oldState);
		switch (S)
		{
		case eHiding:
			CDialogHolder::StartStopMenu(xr_new<PDA>(), true);
			break;
		}
	}

	virtual void OnAnimationEnd(u32 state) override
	{
		inherited::OnAnimationEnd(state);
		
		switch(state)
		{
		case eShowing:
			PDA::SetActiveSubdialog(eptQuests);
			CDialogHolder::StartStopMenu(xr_new<PDA>(), true);
			SwitchState(eIdle);
			break;	// End of Show
		}
	}
	virtual void switch2_Fire() override {}
	virtual void switch2_Fire2() override {}
	virtual void switch2_Reload() override {}

	virtual void OnZoomIn() override {}
	virtual void OnZoomOut() override {}
};