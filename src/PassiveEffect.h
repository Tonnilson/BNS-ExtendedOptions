#pragma once
struct DrEl
{
	char type;
	__int16 subtype;
	unsigned __int16 size;
};

struct _DrEl
{
	DrEl* _el;
};

union EffectRecord_Key
{
	unsigned __int64 key;
	int id;
};

#pragma pack(push,4)
const struct EffectRecord : DrEl
{
	EffectRecord_Key key;
	wchar_t* alias;
	unsigned __int64 name2;
	unsigned __int64 name3;
	__int16 level;
	bool show_icon;
	bool show_left_time;
	bool show_linkbar;
	wchar_t* target_indicator_icon;
	bool target_indicator_show;
	bool show_job_indicator;
	bool save_db;
	char miss_probability;
	int passive_duration;
	int passive_interval;
	int expiration_duration_sec;
	char stack_amount;
	char stack_count;
	bool reattach_effect_after_changing_stack_count;
	__unaligned __declspec(align(2)) unsigned __int64 transform_effect;
	char buff_type;
	char binding_skill_step_type;
	__int16 immune_breaker_attribute;
	__int16 attribute[8];
	__int64 attribute_value;
	__int64 attribute_value_2;
	__int64 attribute_value_3;
	__int16 immune_attribute[8];
	__int64 immune_attribute_value;
	__int64 immune_attribute_value_2;
	__int64 immune_attribute_value_3;
	__int16 flag[4];
	__int16 max_stack_flag;
	char function[10];
	bool drop_field_item;
	bool drop_weapon_field_item;
	char detach_count;
	char detach_slot[4];
	bool apply_duration_formula;
	bool change_default_stance_by_detach_timeout;
	bool target_combat_mode;
	bool aoe_damage;
	__int16 attack_attribute_coefficient_percent;
	char inhalation_linker;
	char inhalation_linked;
	char mount_linker;
	char mount_linked;
	char catch_linker;
	char catch_linked;
	int faction_score_min;
	int faction_score_max;
	bool leave_zone_detach;
	bool not_dead_to_exhaustion_detach;
	bool effect_region_leaved_detach;
	bool invoked_by_effect;
	char event_effect_target[4];
	unsigned __int64 event_effect[4];
	char second_slot_event_effect_target[4];
	__unaligned __declspec(align(1)) unsigned __int64 second_slot_event_effect[4];
	char third_slot_event_effect_target[4];
	unsigned __int64 third_slot_event_effect[4];
	char fourth_slot_event_effect_target[4];
	__unaligned __declspec(align(1)) unsigned __int64 fourth_slot_event_effect[4];
	__int16 modify_ability[8];
	__unaligned __declspec(align(1)) __int64 modify_ability_diff[8];
	__int16 modify_ability_percent[8];
	char passive_moveanim_idle;
	bool pause_auto_targeting;
	bool not_targetable;
	bool is_dot_effect;
	bool no_critical_hit;
	bool party_broadcast;
	char ui_slot;
	char ui_category;
	bool use_extra_skill_stack_count;
	char combat_job[2];
	bool ui_difficult;
	bool is_transform_effect;
	bool is_reuse_standby_effect;
	char item_type;
	bool is_powergauge_effect;
	bool ignore_hide_buff_graph_effect;
	bool is_battle_royal_field_pc_info;
	char battle_royal_field_effect_pouch_group;
	char grocery_effect_type;
	__int16 grocery_effect_level;
	bool knockback_jump;
	char idleanimpriority;
};
#pragma pack(pop)

static_assert(offsetof(EffectRecord, alias) == 0x10);
static_assert(offsetof(EffectRecord, attribute) == 0x58);
static_assert(offsetof(EffectRecord, modify_ability) == 0x16C);
static_assert(offsetof(EffectRecord, ui_slot) == 0x1D2);
static_assert(offsetof(EffectRecord, modify_ability_diff) == 0x17C);
static_assert(offsetof(EffectRecord, modify_ability_percent) == 0x1BC);

struct __declspec(align(4)) DrRecordPtr
{
	_DrEl* _record;
	int _cacheChunkIndex;
};

struct EInterface_Handle
{
	unsigned int Index;
};

struct __declspec(align(4)) Data_EffectRecordPtr : DrRecordPtr
{
	__unaligned __declspec(align(1)) EffectRecord* _debug;
	bool _makeCopy;
};
struct FWindowsPlatformTime {};
struct PreciseTimer
{
	unsigned __int64 _startTime;
	FWindowsPlatformTime _timer;
	float _limit;
};
struct PTPassiveEffectVtbl;

struct PTPassiveEffect
{
	PTPassiveEffectVtbl* vfptr;
	int _effectId;
	int _effectSlot;
	PreciseTimer _timer;
	PreciseTimer _expirationTimer;
	EffectRecord* _effectRecordPtr;
	char pad[0x10];
};

static_assert(offsetof(PTPassiveEffect, _effectRecordPtr) == 0x30);

struct __declspec(align(8)) PTPassiveEffectList
{
	EInterface_Handle _primaryHandle;
	std::vector<PTPassiveEffect> _attachedBuffList;
	std::vector<PTPassiveEffect> _attachedDebuffList;
	std::vector<PTPassiveEffect> _attachedBindingSkillStepList;
	std::vector<PTPassiveEffect> _attachedStaticCountBuffList;
	std::vector<PTPassiveEffect> _attachedSoulMaskList;
};

static_assert(offsetof(PTPassiveEffectList, _attachedDebuffList) == 0x20);
static_assert(offsetof(PTPassiveEffectList, _attachedBindingSkillStepList) == 0x38);

enum CONTEXT_ACTION_TYPE
{
	TYPE_NONE = 0x0,
	TYPE_SKILL = 0x1,
	TYPE_SKILL_SPRINT = 0x2,
	TYPE_SKILL_ACQUIRE_CONDITION = 0x3,
	TYPE_SKILL_SIMPLE_BRANCH_GROUP_SELECT = 0x4,
	TYPE_SKILL_SIMPLE_BRANCH_GROUP_ESCAPE = 0x5,
	TYPE_DUELBOT_CARD = 0x6,
	TYPE_ITEM = 0x7,
	TYPE_GEM_SLOT_ITEM = 0x8,
	TYPE_ITEM_BRAND = 0x9,
	TYPE_QUEST_LOOTING = 0xA,
	TYPE_SKILL_THROW_GADGET = 0xB,
	TYPE_SKILL_USE_GADGET = 0xC,
	TYPE_ACTION_THROW_GADGET = 0xD,
	TYPE_ACTION_DROP_GADGET = 0xE,
	TYPE_ACTION_PICKUP_GADGET = 0xF,
	TYPE_ACTION_PICKUP_DEADBODY = 0x10,
	TYPE_ACTION_OPEN_POUCH = 0x11,
	TYPE_ACTION_OPEN_PRIVATE_POUCH = 0x12,
	TYPE_ACTION_TALK = 0x13,
	TYPE_ACTION_GATHER_SOURCE = 0x14,
	TYPE_ACTION_MANIPULATE = 0x15,
	TYPE_ACTION_PICKUP_ITEM_ALL = 0x16,
	TYPE_ACTION_PICKUP_ITEM_ALL_FROM_ENV = 0x17,
	TYPE_ACTION_AIRDASH = 0x18,
	TYPE_ACTION_AIRDASH_LEAVE = 0x19,
	TYPE_ACTION_ARENA_PORTAL = 0x1A,
	TYPE_ACTION_RESTORATION = 0x1B,
	TYPE_ACTION_RETURN_TO_TOWN = 0x1C,
	TYPE_ACTION_RESURRECTION = 0x1D,
	TYPE_ACTION_HELP_RESTORATION = 0x1E,
	TYPE_ACTION_SUMMON_HELP_RESTORATION = 0x1F,
	TYPE_ACTION_SOCIAL_GROUP = 0x20,
	TYPE_ACTION_SOCIAL = 0x21,
	TYPE_ACTION_SOCIAL_ESCAPE = 0x22,
	TYPE_ACTION_SOCIAL_SELECT_GROUP = 0x23,
	TYPE_ACTION_SOCIAL_EMPTY_QUICK_SLOT_NORMAL = 0x24,
	TYPE_ACTION_SOCIAL_EMPTY_QUICK_SLOT_SPECIAL = 0x25,
	TYPE_MONEY = 0x26,
	TYPE_ACTION_PARTY_MATCH = 0x27,
	TYPE_ACTION_SEXTET_PARTY_MATCH = 0x28,
	TYPE_ACTION_REPAIR = 0x29,
	TYPE_ACTION_RESCUE_ME = 0x2A,
	TYPE_ACTION_SELF_RESURRECT_UNEQIPED = 0x2B,
	TYPE_ACTION_CHECK_PC_INFORMATION = 0x2C,
	TYPE_ACTION_HEART_RECHARGE = 0x2D,
	TYPE_SKILL_LOCKED = 0x2E,
	TYPE_ACTION_SURRENDER = 0x2F,
	TYPE_HUD_CUSTOMIZE_DEFAULT = 0x30,
	TYPE_GUILD_BANK = 0x31,
	TYPE_AUTO_RUN_ON = 0x32,
	TYPE_AUTO_RUN_OFF = 0x33,
	TYPE_MOVE_CAMERA_ON = 0x34,
	TYPE_MOVE_CAMERA_OFF = 0x35,
	TYPE_ATTENDANCE_GOODS = 0x36,
	TYPE_DRAGON_JADE = 0x37,
	TYPE_SKILL_SKIN = 0x38,
	TYPE_FISHING_CANCEL = 0x39,
	TYPE_FISHING_HOOK_SET = 0x3A,
	TYPE_STONE = 0x3B,
	TYPE_ENERGYPOINT_REWARD = 0x3C,
	TYPE_ACTION_GETOFF_VEHICLE = 0x3D,
	TYPE_ACTION_ATTRACTION_POPUP = 0x3E,
	TYPE_COUNT = 0x3F,
};

typedef CONTEXT_ACTION_TYPE PassiveEffectIcon_Type;
typedef CONTEXT_ACTION_TYPE SkillRecycleIcon_Type;

struct EngineEventHandlerVtbl;

struct EngineEventHandler {
	EngineEventHandlerVtbl* vtptr;
};

struct PassiveEffectIcon
{
	float _baseOpacity;
	unsigned __int64 _targetId;
	__int16 _indexId;
	unsigned int _passiveEffectId;
	int _passiveEffectSubType;
	bool _widgetUpdateEnable;
	PassiveEffectIcon_Type _type;
	char _stack_count;
	char _detach_count;
	__int64 _expirationTime;
	bool _show_left_time;
	bool _blink;
	bool _hudCutomizeMode;
	int _startDurationMSec;
	float _leftDuration;
	float MaxPassiveDuration;
	int MaxExpirationDuration;
	unsigned int _iconWidgetId;
	unsigned int _labelWidgetId;
	unsigned int _imageWidgetId;
	unsigned int _nameWidgetId;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _fontset_1;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _fontset_60;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _fontset_600;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _fontset_6000;
	bool _useFontset;
	PreciseTimer _insertedTime;
	PreciseTimer _lastRefreshedTime;
	float _addtionalTime;
	float _tempLeftTime;
	__int64 _effectNameTextId;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _effectNametext;
};

struct __declspec(align(8)) SkillRecycleIcon
{
	float _baseOpacity;
	unsigned __int64 _ownerId;
	uintptr_t* _skillRecordPtr;
	char _skillRecycleGroup;
	char _skillRecycleGroupId;
	bool _widgetUpdateEnable;
	SkillRecycleIcon_Type _type;
	bool _show_left_time;
	bool _blink;
	bool _hudCutomizeMode;
	float _leftDuration;
	unsigned int _iconWidgetId;
	unsigned int _labelWidgetId;
	unsigned int _imageWidgetId;
	unsigned int _nameWidgetId;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _fontset_1;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _fontset_60;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _fontset_600;
	std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > _fontset_6000;
	bool _useFontset;
};

enum PassiveEffectList_EffectVisibleType
{
	DEFAULT_EFFECT_VISIBLETYPE = 0x0,
	ONLY_EVENT_EFFECT_VISIBLETYPE = 0x1,
	NO_EVENT_EFFECT_VISIBLETYPE = 0x2,
};

struct __declspec(align(8)) PassiveEffectList : EngineEventHandler
{
	PassiveEffectIcon BuffBarIcon[35];
	PassiveEffectIcon DebuffBarIcon[35];
	PassiveEffectIcon SystemEffectBarIcon[35];
	PassiveEffectIcon LongTermBarIcon[35];
	PassiveEffectIcon BuffDisableBarIcon[35];
	__int64 _ownerCreatureId;
	char _showBuffAmount;
	SkillRecycleIcon SkillRecycleBarIcon[35];
	PassiveEffectIcon_Type _type;
	bool _hudCustomizeMode;
	bool _autoVisibleHolder;
	bool _enableSort;
	PassiveEffectList_EffectVisibleType _eventBuffVisibleType;
	unsigned int BuffEffectHolder;
	unsigned int DebuffEffectHolder;
	unsigned int DebuffResizeDummy;
	unsigned int SkillRecycleHolder;
	unsigned int BuffDisableHolder;
	unsigned int _systemEffect_holder;
	unsigned int _longTerm_holder;
	bool _useSystemEffect;
	bool _useLongTermEffect;
	bool _updateReserved;
	bool _useBuffGraph;
};

PassiveEffectList* pEffectList = NULL;