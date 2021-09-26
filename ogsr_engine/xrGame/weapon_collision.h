////////////////////////////////////////////////////////////////////////////
//	Файл		: weapon_collision.h
//	Создан		: 12/10/2012
//	Изменен 	: 10.08.21
//	Автор		: lost alpha (SkyLoader)
//	Описание	: Коллизия, стрейфы и т.д. у худа оружия
//	
//	Правки и адаптация под player_hud:
//			i-love-kfc
////////////////////////////////////////////////////////////////////////////
#pragma once

class CWeaponCollision
{
public:
		CWeaponCollision();
		virtual ~CWeaponCollision();
		void Load();
		void Update(Fmatrix &o, float range);
		void CheckState();
private:
		float	fReminderDist;
		float	fReminderWPNUP;
		float	fReminderNeedDist;
		float	fReminderStrafe;
		float	fReminderNeedStrafe;
		float	fReminderMoving;
		float	fReminderNeedMoving;
		float	fReminderNeedWPNUP;
		bool	bFirstUpdate;
		u32	dwMState;
		bool	is_limping;
};