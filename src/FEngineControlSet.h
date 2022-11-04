struct __declspec(align(4)) FEngineControlSet
{
	bool bUseObjectCacheSystem;
	bool bUseAsyncObjectLoader;
	bool bUseResLoadThread;
	bool bRemoveTextureTopMipMap;
	int iRemainTextureMipMapCount;
	bool bShowAutoFrame;
	bool bEndFrameCheck;
	bool bForceApplyCharLOD;
	bool bDisableDynamicLight;
	bool bOptimizeMode;
	bool bOptimizeMode2nd;
	bool bOptimizeOldMode;
	bool bOptimizeMode1;
	bool bOptimizeMode2;
	bool bOptimizeMode3;
	bool bOptimizeMode4;
	float fFXUpdateSkipCount;
	float fFXUpdateFPS;
	bool bShow_PlayerHighEmitter;
	bool bShow_PlayerMidEmitter;
	bool bShow_PlayerLowEmitter;
	bool bShow_PlayerJewelEffect;
	bool bShow_PlayerImmuneEffect;
	bool bShow_PlayerCharLOD;
	bool bShow_PlayerPhysics;
	bool bShow_PlayerParticleLight;
	float fSpawnRate_PlayerFrustum;
	float fSpawnRate_PlayerCullDist;
	float fSpawnRate_PlayerHidden;
	bool bShow_PcHighEmitter;
	bool bShow_PcMidEmitter;
	bool bShow_PcLowEmitter;
	bool bShow_PcJewelEffect;
	bool bShow_PcImmuneEffect;
	bool bShow_PcCharLOD;
	bool bShow_PcPhysics;
	bool bShow_PcParticleLight;
	float fSpawnRate_PcFrustum;
	float fSpawnRate_PcCullDist;
	float fSpawnRate_PcHidden;
	bool bShow_NpcHighEmitter;
	bool bShow_NpcMidEmitter;
	bool bShow_NpcLowEmitter;
	bool bShow_NpcJewelEffect;
	bool bShow_NpcImmuneEffect;
	bool bShow_NpcCharLOD;
	bool bShow_NpcPhysics;
	bool bShow_NpcParticleLight;
	float fSpawnRate_NpcFrustum;
	float fSpawnRate_NpcCullDist;
	float fSpawnRate_NpcHidden;
	bool bShow_BackHighEmitter;
	bool bShow_BackMidEmitter;
	bool bShow_BackLowEmitter;
	bool bShow_BackParticleLight;
	float fSpawnRate_BackFrustum;
	float fSpawnRate_BackCullDist;
	float fSpawnRate_BackHidden;
	float fUC_Hidden;
	int iPawnCount;
	int iParticleCount_High;
	int iParticleCount_Mid;
	int iParticleCount_Low;
	bool bSaveParticlePriorityLog;
};

FEngineControlSet* EffectController;

class BInputKey {
public:
	int Key;
	bool bCtrlPressed;
	bool bShiftPressed;
	bool bAltPressed;
	bool bDoubleClicked;
	bool bTpsModeKey;
};

enum class EngineKeyStateType {
	EKS_PRESSED = 0,
	EKS_RELEASED = 1,
	EKS_REPEAT = 2,
	EKS_DOUBLECLICK = 3,
	EKS_AXIS = 4
};

class EInputKeyEvent {
public:
	char padding[0x18];
	char _vKey;
	char padd_2[0x2];
	EngineKeyStateType KeyState;
	bool bCtrlPressed;
	bool bShiftPressed;
	bool bAltPressed;
};

struct GameSession {
	char pad[0x98];
	float receivedServerPing;
};

struct BNSClient 
{
	char pad[0x48];
	GameSession* game;
	char pad2[0x50];
	uintptr_t* GameWorld;
	char pad3[0x10];
	uintptr_t* PresentationWorld;
};
