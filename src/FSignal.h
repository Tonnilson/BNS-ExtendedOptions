struct FSignalConfig
{
	float DefaultScale;
	float SpaceBetweenWord;
	float PlayerMaxDistance;
	float PlayerMaxScale;
	float PlayerMinScale;
	float PlayerPositionType;
	float ObjMaxDistance;
	float ObjMaxScale;
	float ObjMinScale;
	float ObjPositionType;
	float ComboDistance;
	float ComboOffset;
	unsigned __int32 UseComma : 1;
	int DigitRange;
};

struct USignalInfo
{
	char pad[0x58];
	FSignalConfig Config;
};

struct FDamageFloater {
	char pad[0x18];
	USignalInfo* m_pSignalInfo;
};

uintptr_t SignalInfo_Addr = NULL;
FDamageFloater* pSignalInfo;