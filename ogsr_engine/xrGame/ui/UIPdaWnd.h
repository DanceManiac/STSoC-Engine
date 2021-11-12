#pragma once

#include "UIDialogWnd.h"
#include "UIPdaAux.h"
#include "../encyclopedia_article_defs.h"

class CInventoryOwner;
class CUIFrameLineWnd;
class CUIButton;
class CUITabControl;
class CUIStatic;
class CUIMapWnd;
class CUIEncyclopediaWnd;
class CUIDiaryWnd;
class CUIActorInfoWnd;
class CUIStalkersRankingWnd;
class CUIEventsWnd;
class CUIPdaContactsWnd;
class CUIProgressBar;

class CUIPdaWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd	inherited;
protected:
	//элементы декоративного интерфейса
	CUIFrameLineWnd*		UIMainButtonsBackground;
	CUIFrameLineWnd*		UITimerBackground;

	// кнопки PDA
	CUITabControl*			UITabControl;

	// Установить игровое время
	void					UpdateDateTime					();
	void					DrawUpdatedSections				();
protected:
	// Бэкграунд
	CUIStatic*				UIMainPdaFrame;
	CUIStatic*				m_updatedSectionImage;
	CUIStatic*				m_oldSectionImage;

	// Текущий активный диалог
	CUIWindow*				m_pActiveDialog;
	xr_vector<Fvector2>		m_sign_places_main;

public:
	EPdaTabs				m_pActiveSection;
	// Поддиалоги PDA
	CUIMapWnd*				UIMapWnd;
	CUIPdaContactsWnd*		UIPdaContactsWnd;
	CUIEncyclopediaWnd*		UIEncyclopediaWnd;
	CUIDiaryWnd*			UIDiaryWnd;
	CUIActorInfoWnd*		UIActorInfo;
	CUIStalkersRankingWnd*	UIStalkersRanking;
	CUIEventsWnd*			UIEventsWnd;
	
	virtual void			Reset				();
public:
							CUIPdaWnd			();
	virtual					~CUIPdaWnd			();

	virtual void 			Init				();

	virtual void 			SendMessage			(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual void 			Draw				();
	virtual void 			Update				();
	virtual void 			Show				();
	virtual void 			Hide				();
	virtual bool			OnMouse				(float x, float y, EUIMessages mouse_action);// {CUIDialogWnd::OnMouse(x,y,mouse_action);return true;} //always true because StopAnyMove() == false
	
	virtual void Enable(bool status);
	virtual void MouseMovement(float x, float y);
	
	void					SetActiveSubdialog	(EPdaTabs section);
	virtual bool			StopAnyMove			() { return false; }

	bool bButtonL;
	bool bButtonR;
	u32 dwPDAFrame;

			void			PdaContentsChanged	( pda_section::part type, bool = true );
			
	virtual bool			Action			(s32 cmd, u32 flags);
public:
	/*CUITaskWnd* pUITaskWnd;
	//-	CUIFactionWarWnd*		pUIFactionWarWnd;
	CUIRankingWnd* pUIRankingWnd;
	CUILogsWnd* pUILogsWnd;
	
	CMapSpot* pSelectedMapSpot;*/
public:
	Frect m_cursor_box;
	//UIHint* get_hint_wnd() const { return m_hint_wnd; }
	void DrawHint();

	//virtual void MouseMovement();

	void SetActiveCaption();
	void SetCaption(LPCSTR text);
	void Show_SecondTaskWnd(bool status);
	void Show_MapLegendWnd(bool status);

	void SetActiveDialog(CUIWindow* pUI) { m_pActiveDialog = pUI; };
	CUIWindow* GetActiveDialog() { return m_pActiveDialog; };
	//LPCSTR GetActiveSection() { return m_sActiveSection.c_str(); };
	CUITabControl* GetTabControl() { return UITabControl; };

	void SetActiveSubdialog(const shared_str& section);
	void SetActiveSubdialog_script(LPCSTR section) { SetActiveSubdialog((const shared_str&)section); };

	void UpdatePda();
	void UpdateRankingWnd();
	void ResetCursor();
	float m_power;
	Fvector2 last_cursor_pos;

	Fvector target_joystickrot, joystickrot;
	float target_buttonpress, buttonpress;

	IC void ResetJoystick(bool bForce)
	{
		if (bForce)
		{
			joystickrot.set(0.f, 0.f, 0.f);
			buttonpress = 0.f;
		}

		target_joystickrot.set(0.f, 0.f, 0.f);
		target_buttonpress = 0.f;
	}

DECLARE_SCRIPT_REGISTER_FUNCTION
};
