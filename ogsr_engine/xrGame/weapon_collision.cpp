////////////////////////////////////////////////////////////////////////////
//	Файл		: weapon_collision.cpp
//	Создан		: 12/10/2012
//	Изменен 	: 10.08.21
//	Автор		: lost alpha (SkyLoader)
//	Описание	: Коллизия, стрейфы и т.д. у худа оружия
//	
//	Правки и адаптация под player_hud
//			i-love-kfc
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "weapon_collision.h"
#include "actor.h"
#include "ActorCondition.h"
#include "Weapon.h"

CWeaponCollision::CWeaponCollision()
{
	Load();
}

CWeaponCollision::~CWeaponCollision()
{
}

void CWeaponCollision::Load()
{
	is_limping		= false;
	fReminderDist		= 0;
	fReminderNeedDist	= 0;
	bFirstUpdate		= true;
	fReminderStrafe		= 0.f;
	fReminderNeedStrafe	= 0.f;
	fReminderMoving		= 0.f;
	fReminderNeedMoving	= 0.f;
}

static const float SPEED_REMINDER = 0.62f;
static const float SPEED_REMINDER_STRAFE = 0.5f;
static const float STRAFE_ANGLE = 0.15f;

void CWeaponCollision::CheckState()
{
	dwMState = Actor()->MovingState();
	is_limping = Actor()->conditions().IsLimping();
}

void CWeaponCollision::Update(Fmatrix &o, float range)
{
	//-> Подготавливаем переменные
	CheckState();

	Fvector	xyz	= o.c;
	Fvector	dir;
	o.getHPB(dir.x,dir.y,dir.z);

	if (bFirstUpdate) {
		fReminderDist		= xyz.z;
		fReminderNeedDist	= xyz.z;
		fReminderMoving		= xyz.z;
		fReminderNeedMoving	= xyz.z;
		bFirstUpdate		= false;
	}

    float fYMag = Actor()->fFPCamYawMagnitude;
	//-> Поворот при стрейфе
	if (dwMState&ACTOR_DEFS::mcLStrafe || dwMState&ACTOR_DEFS::mcRStrafe || fYMag != 0.0f)
	{
		float k	= ((dwMState & ACTOR_DEFS::mcCrouch) ? 0.5f : 1.f);
		if (dwMState&ACTOR_DEFS::mcLStrafe || fYMag > 0.f)
			k *= -1;
		if (Actor()->IsZoomAimingMode()) k = 0.f;

		if (isActorAccelerated(dwMState, Actor()->IsZoomAimingMode()))
			fReminderNeedStrafe	= dir.z + (STRAFE_ANGLE * k * 0.9f);
		else if (is_limping)
			fReminderNeedStrafe	= dir.z + (STRAFE_ANGLE * k * 0.75f);
		else
			fReminderNeedStrafe	= dir.z + (STRAFE_ANGLE * k);

	} else fReminderNeedStrafe = 0.f;

	//-> Смещение худа оружия при передвижении вперёд/назад
	if (dwMState&ACTOR_DEFS::mcFwd || dwMState&ACTOR_DEFS::mcBack)
	{
		float f	= ((dwMState & ACTOR_DEFS::mcCrouch) ? 0.5f : 1.f);
		if (dwMState&ACTOR_DEFS::mcFwd)
			f *= -1;

		if (isActorAccelerated(dwMState, Actor()->IsZoomAimingMode()))
			fReminderNeedMoving	= xyz.z + (STRAFE_ANGLE * f * 0.9f * 0.5f);
		else if (is_limping)
			fReminderNeedMoving	= xyz.z + (STRAFE_ANGLE * f * 0.75f * 0.5f);
		else
			fReminderNeedMoving	= xyz.z + (STRAFE_ANGLE * f * 0.5f);

	} else fReminderNeedMoving = 0.f;

	//-> Поворот при стрейфе
	if (!fsimilar(fReminderStrafe, fReminderNeedStrafe)) {
		if (fReminderStrafe < fReminderNeedStrafe)
		{
			fReminderStrafe += SPEED_REMINDER_STRAFE * Device.fTimeDelta;
			if (fReminderStrafe > fReminderNeedStrafe)
				fReminderStrafe = fReminderNeedStrafe;
		} else if (fReminderStrafe > fReminderNeedStrafe)
		{
			fReminderStrafe -= SPEED_REMINDER_STRAFE * Device.fTimeDelta;
			if (fReminderStrafe < fReminderNeedStrafe)
				fReminderStrafe = fReminderNeedStrafe;
		}
	}

	//-> Смещение худа оружия при передвижении вперёд/назад
	if (!fsimilar(fReminderMoving, fReminderNeedMoving)) {
		if (fReminderMoving < fReminderNeedMoving)
		{
			fReminderMoving += SPEED_REMINDER_STRAFE * Device.fTimeDelta;
			if (fReminderMoving > fReminderNeedMoving)
				fReminderMoving = fReminderNeedMoving;
		} else if (fReminderMoving > fReminderNeedMoving)
		{
			fReminderMoving -= SPEED_REMINDER_STRAFE * Device.fTimeDelta;
			if (fReminderMoving < fReminderNeedMoving)
				fReminderMoving = fReminderNeedMoving;
		}
	}

	//-> Высчитываем координаты стрейфа
	if (!fsimilar(fReminderStrafe, dir.z))
	{
		dir.z 		= fReminderStrafe;
		Fmatrix m;
		m.setHPB(dir.x,dir.y,dir.z);
		o.mul_43 (o,m);
	}

	//-> Высчитываем координаты позиции худа оружия
	if (range < 0.8f && !Actor()->IsZoomAimingMode())
		fReminderNeedDist	= xyz.z - ((1 - range - 0.2) * 0.6);
	else if(dwMState&ACTOR_DEFS::mcFwd || dwMState&ACTOR_DEFS::mcBack)
		fReminderNeedDist	= fReminderNeedMoving;
	else
		fReminderNeedDist	= xyz.z;

	if (!fsimilar(fReminderDist, fReminderNeedDist)) {
		if (fReminderDist < fReminderNeedDist) {
			fReminderDist += SPEED_REMINDER * Device.fTimeDelta;
			if (fReminderDist > fReminderNeedDist)
				fReminderDist = fReminderNeedDist;
		} else if (fReminderDist > fReminderNeedDist) {
			fReminderDist -= SPEED_REMINDER * Device.fTimeDelta;
			if (fReminderDist < fReminderNeedDist)
				fReminderDist = fReminderNeedDist;
		}
	}

	if (!fsimilar(fReminderDist, xyz.z))
	{
		xyz.z = fReminderDist;
		o.c.set(xyz);
	}
}