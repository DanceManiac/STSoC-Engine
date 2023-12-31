#pragma once
#include "firedeps.h"

#include "../Include/xrRender/Kinematics.h"
#include "../Include/xrRender/KinematicsAnimated.h"
#include "actor_defs.h"
#include "weapon_collision.h"

class player_hud;
class CHudItem;
class CMotionDef;
struct attachable_hud_item;

struct motion_descr
{
	MotionID mid;
	shared_str name;
	float speed_k{ 1.0f };
	float stop_k{ 1.0f };
	const char* eff_name{};
};

struct player_hud_motion
{
	shared_str m_alias_name;
	shared_str m_base_name;
	shared_str m_additional_name;
	xr_vector<motion_descr> m_animations;
};

struct player_hud_motion_container
{
	xr_vector<player_hud_motion> m_anims;
	player_hud_motion* find_motion(const shared_str& name);
	void load(attachable_hud_item* parent, IKinematicsAnimated* model, IKinematicsAnimated* animatedHudItem, const shared_str& sect);
};

struct hud_item_measures
{
	enum
	{
		e_fire_point = (1 << 0),
		e_fire_point2 = (1 << 1),
		e_shell_point = (1 << 2),
		e_16x9_mode_now = (1 << 3)
	};
	void merge_measures_params();
	Flags8 m_prop_flags;

	Fvector m_item_attach[2]{}; // pos,rot


	enum m_hands_offset_coords : u8 {
		m_hands_offset_pos,
		m_hands_offset_rot,
		m_hands_offset_size
	};
	enum m_hands_offset_type : u8 {
		m_hands_offset_type_normal, // Не прицеливаемся
		m_hands_offset_type_aim, // Смотрим в механический прицел
		m_hands_offset_type_aim_alt, // Смотрим в альтернативный прицел
		m_hands_offset_type_gl, // Смотрим в механический прицел в режиме ПГ
		m_hands_offset_type_aim_scope, // Смотрим в присоединяемый нетекстурный прицел (будь то 3д прицел или колиматор) если включен "use_scope_zoom"
		m_hands_offset_type_gl_scope, // Смотрим в присоединяемый нетекстурный прицел (будь то 3д прицел или колиматор) в режиме ПГ если включен "use_scope_grenade_zoom" - мне вот щас не понятно зачем это надо, но это как-то используют.
		m_hands_offset_type_aim_gl_normal, // Смотрим в механический прицел если гранатомет присоединен
		m_hands_offset_type_gl_normal_scope, // Смотрим в присоединяемый нетекстурный прицел (будь то 3д прицел или колиматор) если включен "use_scope_zoom" и гранатомет присоединен
		m_hands_offset_type_size
	};
	Fvector m_hands_offset[m_hands_offset_size][m_hands_offset_type_size]{};
	bool AltAimAllowed{};
	bool AltAimEnabled{};

	IC void SwitchAimType() {
		AltAimEnabled = (AltAimEnabled || !AltAimAllowed) ? false : true;
	}

	IC const bool IsAltAimEnabled() {
		return AltAimAllowed && AltAimEnabled;
	}

	u16 m_fire_bone;
	Fvector m_fire_point_offset;
	u16 m_fire_bone2;
	Fvector m_fire_point2_offset;
	u16 m_shell_bone;
	Fvector m_shell_point_offset;

	Fvector m_hands_attach[4]{}; // pos,rot

	void load(const shared_str& sect_name, IKinematics* K);
    bool bReloadShooting; //--#SM+#--

	struct inertion_params
	{
		float m_pitch_offset_r;
		float m_pitch_offset_n;
		float m_pitch_offset_d;
		float m_pitch_low_limit;
		// отклонение модели от "курса" из за инерции во время движения
		float m_origin_offset;
		// отклонение модели от "курса" из за инерции во время движения с прицеливанием
		float m_origin_offset_aim;
		// скорость возврата худ модели в нужное положение
		float m_tendto_speed;
		// скорость возврата худ модели в нужное положение во время прицеливания
		float m_tendto_speed_aim;
	};
	inertion_params m_inertion_params; //--#SM+#--	
	
    struct shooting_params
    {
        Fvector4 m_shot_max_offset_LRUD;
        Fvector4 m_shot_max_offset_LRUD_aim;
        Fvector2 m_shot_offset_BACKW;
        float m_ret_speed;
        float m_ret_speed_aim;
        float m_min_LRUD_power;
    };
    shooting_params m_shooting_params; //--#SM+#--
};

struct attachable_hud_item
{
	player_hud* m_parent;
	CHudItem* m_parent_hud_item{};
	shared_str m_sect_name;
	shared_str m_visual_name;
	IKinematics* m_model{};
	u16 m_attach_place_idx{};
	bool m_bad_inertion_fix;
	hud_item_measures m_measures;

	// runtime positioning
	Fmatrix m_attach_offset;
	Fmatrix m_item_transform;

	player_hud_motion_container m_hand_motions;

	attachable_hud_item(player_hud* pparent) : m_parent(pparent), m_upd_firedeps_frame(u32(-1)) {}
	~attachable_hud_item();

	void load(const shared_str& sect_name);
	void update(bool bForce);
	void update_hud_additional(Fmatrix& trans);
	void setup_firedeps(firedeps& fd);
	void render();
	void render_item_ui();
	bool render_item_ui_query();
	bool need_renderable();
	void set_bone_visible(const shared_str& bone_name, BOOL bVisibility, BOOL bSilent = FALSE);
	BOOL get_bone_visible(const shared_str& bone_name);
	bool has_bone(const shared_str& bone_name);
	void debug_draw_firedeps();

	// hands bind position
	Fvector& hands_attach_pos(u8 = 0);
	Fvector& hands_attach_rot(u8 = 0);

	// hands runtime offset
	Fvector& hands_offset_pos();
	Fvector& hands_offset_rot();

	// props
	u32 m_upd_firedeps_frame;
	void tune(Ivector values);
	u32 anim_play(const shared_str& anim_name, BOOL bMixIn, const CMotionDef*& md, u8& rnd, bool randomAnim);
};

class CWeaponBobbing
{
public:
	CWeaponBobbing();
	~CWeaponBobbing() = default;

	void Load();
	void Update(Fmatrix& m, attachable_hud_item* hi);
	void CheckState();

private:
	float	fTime;
	Fvector	vAngleAmplitude;
	float	fYAmplitude;
	float	fSpeed;

	u32		dwMState;
	float	fReminderFactor;
	bool	is_limping;
	bool	m_bZoomMode;

	float	m_fAmplitudeRun;
	float	m_fAmplitudeWalk;
	float	m_fAmplitudeLimp;

	float	m_fSpeedRun;
	float	m_fSpeedWalk;
	float	m_fSpeedLimp;

	float	m_fCrouchFactor;
	float	m_fZoomFactor;
	float	m_fScopeZoomFactor;
};

enum eMovementLayers
{
	eAimWalk = 0,
	eAimCrouch,
	eCrouch,
	eWalk,
	eRun,
	eSprint,
	move_anms_end
};

#include "../xr_3da/ObjectAnimator.h"
struct movement_layer
{
	CObjectAnimator* anm;
	float blend_amount[2];
	bool active;
	float m_power;
	Fmatrix blend;
	u8 m_part;

	movement_layer()
	{
		blend.identity();
		anm = xr_new<CObjectAnimator>();
		blend_amount[0] = 0.f;
		blend_amount[1] = 0.f;
		active = false;
		m_power = 1.f;
	}

	void Load(LPCSTR name)
	{
		if (xr_strcmp(name, anm->Name()))
			anm->Load(name);
	}

	void Play(bool bLoop = true)
	{
		if (!anm->Name())
			return;

		if (IsPlaying())
		{
			active = true;
			return;
		}
		
		anm->Play(bLoop);
		active = true;
	}

	bool IsPlaying()
	{
		return anm->IsPlaying();
	}

	void Stop(bool bForce)
	{
		if (bForce)
		{
			anm->Stop();
			blend_amount[0] = 0.f;
			blend_amount[1] = 0.f;
			blend.identity();
		}

		active = false;
	}

	const Fmatrix& XFORM(u8 part)
	{
		blend.set(anm->XFORM());
		blend.mul(blend_amount[part] * m_power);
		blend.m[0][0] = 1.f;
		blend.m[1][1] = 1.f;
		blend.m[2][2] = 1.f;

		return blend;
	}
};

class player_hud
{
public:
	player_hud();
	~player_hud();
	void load(const shared_str& model_name);
	void load_default() { load("actor_hud_05"); };
	void update(const Fmatrix& trans);
	void render_hud();
	void render_item_ui();
	bool render_item_ui_query();
	u32 anim_play(u16 part, const /*motion_descr&*/MotionID& M, BOOL bMixIn, const CMotionDef*& md, float speed, IKinematicsAnimated* itemModel = nullptr, u16 override_part = u16(-1));
	const shared_str& section_name() const { return m_sect_name; }
	attachable_hud_item* create_hud_item(const shared_str& sect);

	void attach_item(CHudItem* item);
	void re_sync_anim(u8 part);
	bool allow_activation(CHudItem* item);
	attachable_hud_item* attached_item(u16 item_idx) { return m_attached_items[item_idx]; };
	void detach_item_idx(u16 idx);
	void detach_item(CHudItem* item);
	void detach_all_items()
	{
		m_attached_items[0] = nullptr;
		m_attached_items[1] = nullptr;
	}

	void calc_transform(u16 attach_slot_idx, const Fmatrix& offset, Fmatrix& result);
	void tune(Ivector values);
	u32 motion_length(const motion_descr& M, const CMotionDef*& md, float speed, IKinematicsAnimated* itemModel, attachable_hud_item* pi = nullptr);
	u32 motion_length(const shared_str& anim_name, const shared_str& hud_name, const CMotionDef*& md);
	void OnMovementChanged(ACTOR_DEFS::EMoveCommand cmd);
	void updateMovementLayerState();

	xr_vector<movement_layer*> m_movement_layers;
private:
	void update_inertion(Fmatrix& trans);
	void update_additional(Fmatrix& trans);
	bool inertion_allowed();
	bool bobbing_allowed();
	bool collision_allowed();

private:
	const Fvector& attach_rot(u8 part) const;
	const Fvector& attach_pos(u8 part) const;

	shared_str m_sect_name;

	Fmatrix m_attach_offset;
	Fmatrix m_attach_offset_2;

public:
	Fmatrix m_transform;
	Fmatrix m_transform_2;
private:
	IKinematicsAnimated* m_model{};
	IKinematicsAnimated* m_model_2{};
	xr_vector<u16> m_ancors;
	attachable_hud_item* m_attached_items[2]{};
	xr_vector<attachable_hud_item*> m_pool;
	CWeaponBobbing* m_bobbing{};

	private:
		CWeaponCollision *m_collision;
};

extern player_hud* g_player_hud;
