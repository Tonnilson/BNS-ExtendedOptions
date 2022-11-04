// dllmain.cpp : Defines the entry point for the DLL application.

#include <pe/module.h>
#include <xorstr/include/xorstr.hpp>
#include <pluginsdk.h>
#include <searchers.h>
#include "pugixml/src/pugixml.hpp"
#include "FEngineControlSet.h"
#include <map>
#include <ShlObj.h>
#include <KnownFolders.h>
#include <filesystem>
#include <wil/include/wil/stl.h>
#include <wil/include/wil/win32_helpers.h>
#include <Detours/include/detours.h>
#include <locale>
#include <codecvt>
#include <string>
#include "PassiveEffect.h"
#include "FSignal.h"

std::filesystem::path docPath;
std::map<std::string, uintptr_t> FXMap;
pugi::xml_document CfgDoc;
uintptr_t* DataManager_Instance;
int DataManager_EffectRecord = 0;
int PassiveEffectList_Offset = 0;

BInputKey ReloadInput;
BInputKey Profile_1;
BInputKey Profile_2;
BInputKey Profile_3;
BInputKey Profile_4;
BInputKey Profile_5;

bool bEnablePhantomWeapon = true;

struct World;
World* (__fastcall* BNSClient_GetWorld)();
void(__fastcall* ExecuteConsoleCommandNoHistory)(const wchar_t* szCmd);
void(__fastcall* AddInstantNotification)(World* thisptr,
	const wchar_t* text,
	const wchar_t* particleRef,
	const wchar_t* sound,
	char track,
	bool stopPreviousSound,
	bool headline2,
	bool boss_headline,
	bool chat,
	char category,
	const wchar_t* sound2);

// Just a wrapper for AddInstantNotification
void __fastcall AddNotification(const wchar_t* text,
	const wchar_t* particleRef,
	const wchar_t* sound,
	char track,
	bool stopPreviousSound,
	bool headline2,
	bool boss_headline,
	bool chat,
	char category,
	const wchar_t* sound2) {
	auto World = BNSClient_GetWorld();
	if (World && AddInstantNotification)
		AddInstantNotification(World, text, particleRef, sound, track, stopPreviousSound, headline2, boss_headline, chat, category, sound2);
}

struct UiStateGame;
uintptr_t(__fastcall* GetUiStateGame)();

#define QueryEffectRecord(effectId) (*(EffectRecord * (__fastcall**)(uintptr_t, uintptr_t))(**(uintptr_t**)(*DataManager_Instance + DataManager_EffectRecord) + 0xB8))(*(uintptr_t*)(*DataManager_Instance + DataManager_EffectRecord),effectId)

void ConsoleWrite(const wchar_t* msg, ...) {
	wchar_t szBuffer[1024];
	va_list args;
	va_start(args, msg);
	vswprintf(szBuffer, 1024, msg, args);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), szBuffer, wcslen(szBuffer), NULL, NULL);
	va_end(args);
}

const std::filesystem::path& documents_path()
{
	static std::once_flag once_flag;
	static std::filesystem::path path;

	std::call_once(once_flag, [](std::filesystem::path& path) {
		wil::unique_cotaskmem_string result;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &result)))
			path = result.get();
		}, path);
	return path;
}

uintptr_t GetAddress(uintptr_t AddressOfCall, int index, int length)
{
	if (!AddressOfCall)
		return 0;

	long delta = *(long*)(AddressOfCall + index);
	return (AddressOfCall + delta + length);
}

// This is weird, could use std::format and some other stuff but yeah makes it kind of chunky
int GetKeyCodeFromString(const pugi::char_t* str) {
	std::string key = xorstr_("0x");
	key += str;
	return std::stoi(key, nullptr, 0);
}

void ConfigureFXProfile(int profile_id) {
	std::string video_query = xorstr_("/config/profile_") + std::to_string(profile_id) + xorstr_("/video_options/option");
	std::string cmds_query = xorstr_("/config/profile_") + std::to_string(profile_id) + xorstr_("/console_cmds/cmd");
	std::string phantom_query = xorstr_("/config/profile_") + std::to_string(profile_id) + xorstr_("/phantom");
	std::string font_query = xorstr_("/config/profile_") + std::to_string(profile_id) + xorstr_("/damage_font");

	bEnablePhantomWeapon = CfgDoc.select_node(phantom_query.c_str()).node().attribute(xorstr_("enable")).as_bool(true);
	auto nodes = CfgDoc.select_nodes(video_query.c_str());
	for (auto node : nodes) {
		auto nodeName = node.node().attribute(xorstr_("name")).as_string();
		if (FXMap.count(nodeName) > 0) {
			*reinterpret_cast<bool*>(FXMap[nodeName]) = node.node().attribute(xorstr_("enable")).as_bool(true);
		}
	}

	auto cmds = CfgDoc.select_nodes(cmds_query.c_str());
	for (auto cmd : cmds) {
		std::string mycmd = (cmd.node().attribute(xorstr_("run")).as_string());
		std::wstring runcmd(mycmd.begin(), mycmd.end());
		ExecuteConsoleCommandNoHistory(runcmd.c_str());
	}

	if (!pSignalInfo)
		pSignalInfo = *(FDamageFloater**)SignalInfo_Addr;

	if (pSignalInfo) {
		pSignalInfo->m_pSignalInfo->Config.DefaultScale = CfgDoc.select_node(font_query.c_str()).node().attribute(xorstr_("scale")).as_float(1.6f);
		pSignalInfo->m_pSignalInfo->Config.SpaceBetweenWord = CfgDoc.select_node(font_query.c_str()).node().attribute(xorstr_("wordspacing")).as_float(0.0f);;
	}
}

void __fastcall initSystem() {
	FXMap.insert(std::make_pair(xorstr_("PlayerHighEmitter"), (uintptr_t)&EffectController->bShow_PlayerHighEmitter));
	FXMap.insert(std::make_pair(xorstr_("PlayerMidEmitter"), (uintptr_t)&EffectController->bShow_PlayerMidEmitter));
	FXMap.insert(std::make_pair(xorstr_("PlayerLowEmitter"), (uintptr_t)&EffectController->bShow_PlayerLowEmitter));
	FXMap.insert(std::make_pair(xorstr_("PlayerJewelEffect"), (uintptr_t)&EffectController->bShow_PlayerJewelEffect));
	FXMap.insert(std::make_pair(xorstr_("PlayerImmuneEffect"), (uintptr_t)&EffectController->bShow_PlayerImmuneEffect));
	FXMap.insert(std::make_pair(xorstr_("PlayerCharLOD"), (uintptr_t)&EffectController->bShow_PlayerCharLOD));
	FXMap.insert(std::make_pair(xorstr_("PlayerPhysics"), (uintptr_t)&EffectController->bShow_PlayerPhysics));
	FXMap.insert(std::make_pair(xorstr_("PlayerParticleLight"), (uintptr_t)&EffectController->bShow_PlayerParticleLight));

	// Other Player characters
	FXMap.insert(std::make_pair(xorstr_("PcHighEmitter"), (uintptr_t)&EffectController->bShow_PcHighEmitter));
	FXMap.insert(std::make_pair(xorstr_("PcMidEmitter"), (uintptr_t)&EffectController->bShow_PcMidEmitter));
	FXMap.insert(std::make_pair(xorstr_("PcLowEmitter"), (uintptr_t)&EffectController->bShow_PcLowEmitter));
	FXMap.insert(std::make_pair(xorstr_("PcJewelEffect"), (uintptr_t)&EffectController->bShow_PcJewelEffect));
	FXMap.insert(std::make_pair(xorstr_("PcImmuneEffect"), (uintptr_t)&EffectController->bShow_PcImmuneEffect));
	FXMap.insert(std::make_pair(xorstr_("PcCharLOD"), (uintptr_t)&EffectController->bShow_PcCharLOD));
	FXMap.insert(std::make_pair(xorstr_("PcPhysics"), (uintptr_t)&EffectController->bShow_PcPhysics));
	FXMap.insert(std::make_pair(xorstr_("PcParticleLight"), (uintptr_t)&EffectController->bShow_PcParticleLight));

	// Boss Stuff
	FXMap.insert(std::make_pair(xorstr_("NpcHighEmitter"), (uintptr_t)&EffectController->bShow_NpcHighEmitter));
	FXMap.insert(std::make_pair(xorstr_("NpcMidEmitter"), (uintptr_t)&EffectController->bShow_NpcMidEmitter));
	FXMap.insert(std::make_pair(xorstr_("NpcLowEmitter"), (uintptr_t)&EffectController->bShow_NpcLowEmitter));
	FXMap.insert(std::make_pair(xorstr_("NpcJewelEffect"), (uintptr_t)&EffectController->bShow_NpcJewelEffect));
	FXMap.insert(std::make_pair(xorstr_("NpcImmuneEffect"), (uintptr_t)&EffectController->bShow_NpcImmuneEffect));
	FXMap.insert(std::make_pair(xorstr_("NpcCharLOD"), (uintptr_t)&EffectController->bShow_NpcCharLOD));
	FXMap.insert(std::make_pair(xorstr_("NpcPhysics"), (uintptr_t)&EffectController->bShow_NpcPhysics));
	FXMap.insert(std::make_pair(xorstr_("NpcParticleLight"), (uintptr_t)&EffectController->bShow_NpcParticleLight));

	// Background Stuff
	FXMap.insert(std::make_pair(xorstr_("BackHighEmitter"), (uintptr_t)&EffectController->bShow_BackHighEmitter));
	FXMap.insert(std::make_pair(xorstr_("BackMidEmitter"), (uintptr_t)&EffectController->bShow_BackMidEmitter));
	FXMap.insert(std::make_pair(xorstr_("BackLowEmitter"), (uintptr_t)&EffectController->bShow_BackLowEmitter));
	FXMap.insert(std::make_pair(xorstr_("BackParticleLight"), (uintptr_t)&EffectController->bShow_BackParticleLight));
	ConfigureFXProfile(0);
}

bool bInit = false;
void(__fastcall* oSystemFunc_SetOutFrustumParticleSpawnRate)(uintptr_t* thisptr, uintptr_t* eType, const float* fRate);
void __fastcall  hkSystemFunc_SetOutFrustumParticleSpawnRate(uintptr_t* thisptr, uintptr_t* eType, const float* fRate) {
	if (!bInit) {
		initSystem();
		bInit = true;
	}
	return oSystemFunc_SetOutFrustumParticleSpawnRate(thisptr, eType, fRate);
}

// Why is this needed? It's so it does not keep firing off per tick.
bool ReloadConfig = false;
bool Profile_1_Pressed = false;
bool Profile_2_Pressed = false;
bool Profile_3_Pressed = false;
bool Profile_4_Pressed = false;
bool Profile_5_Pressed = false;
bool Print_Active_Effects = false;
void(__fastcall* oBInputKey)(BInputKey* thisptr, EInputKeyEvent* InputKeyEvent);
void __fastcall hkBInputKey(BInputKey* thisptr, EInputKeyEvent* InputKeyEvent) {

	if (InputKeyEvent->_vKey == ReloadInput.Key) {
		if (InputKeyEvent->bAltPressed == ReloadInput.bAltPressed &&
			InputKeyEvent->bCtrlPressed == ReloadInput.bCtrlPressed &&
			InputKeyEvent->bShiftPressed == ReloadInput.bShiftPressed)
		{
			if (!ReloadConfig && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
				ReloadConfig = true;
				CfgDoc.load_file(docPath.c_str(), pugi::parse_default);

				auto reload_key = (CfgDoc.select_node(xorstr_("/config/options/option[@name='reloadKey']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("39"))); // Default is 6 key
				ReloadInput.Key = GetKeyCodeFromString(reload_key);
				ReloadInput.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='reloadKey']")).node().attribute(xorstr_("bAlt")).as_bool(true));
				ReloadInput.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='reloadKey']")).node().attribute(xorstr_("bCtrl")).as_bool());
				ReloadInput.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='reloadKey']")).node().attribute(xorstr_("bShift")).as_bool());

				auto profile_1 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_1']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("31"))); // Default is 6 key
				Profile_1.Key = GetKeyCodeFromString(profile_1);
				Profile_1.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_1']")).node().attribute(xorstr_("bAlt")).as_bool(true));
				Profile_1.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_1']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
				Profile_1.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_1']")).node().attribute(xorstr_("bShift")).as_bool());

				auto profile_2 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_2']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("32"))); // Default is 6 key
				Profile_2.Key = GetKeyCodeFromString(profile_2);
				Profile_2.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_2']")).node().attribute(xorstr_("bAlt")).as_bool(true));
				Profile_2.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_2']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
				Profile_2.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_2']")).node().attribute(xorstr_("bShift")).as_bool());

				auto profile_3 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_3']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("33"))); // Default is 6 key
				Profile_3.Key = GetKeyCodeFromString(profile_3);
				Profile_3.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_3']")).node().attribute(xorstr_("bAlt")).as_bool(true));
				Profile_3.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_3']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
				Profile_3.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_3']")).node().attribute(xorstr_("bShift")).as_bool());

				auto profile_4 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_4']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("33"))); // Default is 6 key
				Profile_4.Key = GetKeyCodeFromString(profile_4);
				Profile_4.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_4']")).node().attribute(xorstr_("bAlt")).as_bool(true));
				Profile_4.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_4']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
				Profile_4.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_4']")).node().attribute(xorstr_("bShift")).as_bool());

				auto profile_5 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_5']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("33"))); // Default is 6 key
				Profile_5.Key = GetKeyCodeFromString(profile_5);
				Profile_5.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_5']")).node().attribute(xorstr_("bAlt")).as_bool(true));
				Profile_5.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_5']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
				Profile_5.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_5']")).node().attribute(xorstr_("bShift")).as_bool());

				ConfigureFXProfile(0);

				AddNotification(xorstr_(L"Extended Options Reloaded"), L"", L"", 0, false, false, false, true, 0x16, L"");
			}
			else if (ReloadConfig && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
				ReloadConfig = false;
		}
		else
			ReloadConfig = false;
	}
	else if (InputKeyEvent->_vKey == Profile_1.Key) {
		if (InputKeyEvent->bAltPressed == Profile_1.bAltPressed &&
			InputKeyEvent->bCtrlPressed == Profile_1.bCtrlPressed &&
			InputKeyEvent->bShiftPressed == Profile_1.bShiftPressed)
		{
			if (!Profile_1_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
				Profile_1_Pressed = true;
				ConfigureFXProfile(1);
				AddNotification(xorstr_(L"ExtendedOptions: Using Profile 1"), L"", L"", 0, false, false, false, true, 0x16, L"");
			}
			else if (Profile_1_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
				Profile_1_Pressed = false;
		}
		else
			Profile_1_Pressed = false;
	}
	else if (InputKeyEvent->_vKey == Profile_2.Key) {
		if (InputKeyEvent->bAltPressed == Profile_2.bAltPressed &&
			InputKeyEvent->bCtrlPressed == Profile_2.bCtrlPressed &&
			InputKeyEvent->bShiftPressed == Profile_2.bShiftPressed) {
			if (!Profile_2_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
				Profile_2_Pressed = true;
				ConfigureFXProfile(2);
				AddNotification(xorstr_(L"ExtendedOptions: Using Profile 2"), L"", L"", 0, false, false, false, true, 0x16, L"");
			}
			else if (Profile_2_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
				Profile_2_Pressed = false;
		}
		else
			Profile_2_Pressed = false;
	}
	else if (InputKeyEvent->_vKey == Profile_3.Key) {
		if (InputKeyEvent->bAltPressed == Profile_3.bAltPressed &&
			InputKeyEvent->bCtrlPressed == Profile_3.bCtrlPressed &&
			InputKeyEvent->bShiftPressed == Profile_3.bShiftPressed) {
			if (!Profile_3_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
				Profile_3_Pressed = true;
				ConfigureFXProfile(3);
				AddNotification(xorstr_(L"ExtendedOptions: Using Profile 3"), L"", L"", 0, false, false, false, true, 0x16, L"");
			}
			else if (Profile_3_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
				Profile_3_Pressed = false;
		}
		else
			Profile_3_Pressed = false;
	}
	else if (InputKeyEvent->_vKey == Profile_4.Key) {
		if (InputKeyEvent->bAltPressed == Profile_4.bAltPressed &&
			InputKeyEvent->bCtrlPressed == Profile_4.bCtrlPressed &&
			InputKeyEvent->bShiftPressed == Profile_4.bShiftPressed) {
			if (!Profile_4_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
				Profile_4_Pressed = true;
				ConfigureFXProfile(4);
				AddNotification(xorstr_(L"ExtendedOptions: Using Profile 4"), L"", L"", 0, false, false, false, true, 0x16, L"");
			}
			else if (Profile_4_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
				Profile_4_Pressed = false;
		}
		else
			Profile_4_Pressed = false;
	}
	else if (InputKeyEvent->_vKey == Profile_5.Key) {
		if (InputKeyEvent->bAltPressed == Profile_5.bAltPressed &&
			InputKeyEvent->bCtrlPressed == Profile_5.bCtrlPressed &&
			InputKeyEvent->bShiftPressed == Profile_5.bShiftPressed) {
			if (!Profile_5_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
				Profile_5_Pressed = true;
				ConfigureFXProfile(5);
				AddNotification(xorstr_(L"ExtendedOptions: Using Profile 5"), L"", L"", 0, false, false, false, true, 0x16, L"");
			}
			else if (Profile_5_Pressed && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
				Profile_5_Pressed = false;
		}
		else
			Profile_5_Pressed = false;
	}
	else if (InputKeyEvent->_vKey == 0x30) {
		if (InputKeyEvent->bAltPressed &&
			InputKeyEvent->bCtrlPressed &&
			!InputKeyEvent->bShiftPressed)
		{
			if (!Print_Active_Effects && InputKeyEvent->KeyState == EngineKeyStateType::EKS_PRESSED) {
				Print_Active_Effects = true;
				if (GetConsoleWindow()) {
					auto UiState = GetUiStateGame();
					if (UiState && PassiveEffectList_Offset) {
						auto& _effectList = *reinterpret_cast<PassiveEffectList**>(UiState + PassiveEffectList_Offset);
						ConsoleWrite(xorstr_(L"\n----------------------------\nBuff Bar\n----------------------------\n"));
						for (auto& x : _effectList->BuffBarIcon) {
							if (x._passiveEffectId != 0) {
								auto _effectRecord = QueryEffectRecord(x._passiveEffectId);

								if (_effectRecord) {
									ConsoleWrite(xorstr_(L"Alias: %s | category: %x\n"), _effectRecord->alias, _effectRecord->ui_category);
								}
							}
						}

						ConsoleWrite(xorstr_(L"\n----------------------------\nDebuff Bar\n----------------------------\n"));
						for (auto& x : _effectList->DebuffBarIcon) {
							if (x._passiveEffectId != 0) {
								auto _effectRecord = QueryEffectRecord(x._passiveEffectId);

								if (_effectRecord) {
									ConsoleWrite(xorstr_(L"Alias: %s | category: %x\n"), _effectRecord->alias, _effectRecord->ui_category);
								}
							}
						}

						ConsoleWrite(xorstr_(L"\n----------------------------\nSystem Bar\n----------------------------\n"));
						for (auto& x : _effectList->SystemEffectBarIcon) {
							if (x._passiveEffectId != 0) {
								auto _effectRecord = QueryEffectRecord(x._passiveEffectId);

								if (_effectRecord) {
									ConsoleWrite(xorstr_(L"Alias: %s | category: %x\n"), _effectRecord->alias, _effectRecord->ui_category);
								}
							}
						}

						ConsoleWrite(xorstr_(L"\n----------------------------\nLong-term Bar\n----------------------------\n"));
						for (auto& x : _effectList->LongTermBarIcon) {
							if (x._passiveEffectId != 0) {
								auto _effectRecord = QueryEffectRecord(x._passiveEffectId);

								if (_effectRecord) {
									ConsoleWrite(xorstr_(L"Alias: %s | category: %x\n"), _effectRecord->alias, _effectRecord->ui_category);
								}
							}
						}

						ConsoleWrite(xorstr_(L"\n----------------------------\nBuff-Disable Bar\n----------------------------\n"));
						for (auto& x : _effectList->BuffDisableBarIcon) {
							if (x._passiveEffectId != 0) {
								auto _effectRecord = QueryEffectRecord(x._passiveEffectId);

								if (_effectRecord) {
									ConsoleWrite(xorstr_(L"Alias: %s | category: %x\n"), _effectRecord->alias, _effectRecord->ui_category);
								}
							}
						}
					}
				}
			}
			else if (Print_Active_Effects && InputKeyEvent->KeyState == EngineKeyStateType::EKS_RELEASED)
				Print_Active_Effects = false;
		}
		else
			Print_Active_Effects = false;
	}
	return oBInputKey(thisptr, InputKeyEvent);
}

// This is when an effect is to be added to the status bars, we hook this to intercept before it is placed and sorted
char(__fastcall* oPassiveEffectList_add)(uintptr_t* thisptr, unsigned __int64 targetId, __int16 effectSlotIndex, unsigned int effectId, int duration, __int64 expirationTime, float MaxDuration, char stack_count, char detach_count);
char __fastcall hkPassiveEffectList_add(uintptr_t* thisptr, unsigned __int64 targetId, __int16 effectSlotIndex, unsigned int effectId, int duration, __int64 expirationTime, float MaxDuration, char stack_count, char detach_count) {
	// Something that was in the original to end operations early
	if ((effectSlotIndex & 0xFF00) == 512)
		return 0;

	/*
		Query the effect record for the specific effectId
		Returns pointer to effect record or 0 if it does not exist / is cached.

		The function should cache the record if its not already cached...? So the only reason why it should return 0 is that the specific effectId does not exist
	*/
	auto _effectRecord = QueryEffectRecord(effectId);

	// EffectRecord is cached and available
	if (_effectRecord) {
		// Note to self, pugi is fast but it's still parsing an XML. Performance issues could potentially become a problem with a lot of records, should probably look into a better system in the future like using a hashmap
		std::wstring effect_query = xorstr_(L"/config/effect_list/effect[@name='"); effect_query += _effectRecord->alias; effect_query += xorstr_(L"']"); // Yes this is dumb but xorstr_ cannot be used with std::format
		pugi::xpath_query query(std::string(effect_query.begin(), effect_query.end()).c_str()); // Also dumb, should just turn pugi widechar mode on
		pugi::xpath_node result = CfgDoc.select_node(query);
		
		// If there's a match make the changes that the user wants
		if (result) {
			if(result.node().attribute(xorstr_("ui-slot")).as_int(20) != 20)
				_effectRecord->ui_slot = (char)result.node().attribute(xorstr_("ui-slot")).as_int();
			if (result.node().attribute(xorstr_("ui-category")).as_int(20) != 20)
				_effectRecord->ui_category = (char)result.node().attribute(xorstr_("ui-category")).as_int();
		}
	}

	// Continue running original code
	return oPassiveEffectList_add(thisptr, targetId, effectSlotIndex, effectId, duration, expirationTime, MaxDuration, stack_count, detach_count);
}

void(__fastcall* oClearPhantomMode)(uintptr_t* thisptr);
void(__fastcall* oUpdatePhantomWeaponMode)(uintptr_t* thisptr, bool isTimeOutDetah);
void __fastcall hkUpdatePhantomWeaponMode(uintptr_t* thisptr, bool isTimeOutDetah) {
	if(!bEnablePhantomWeapon)
		return oClearPhantomMode(thisptr);

	return oUpdatePhantomWeaponMode(thisptr, isTimeOutDetah);
}

void(__fastcall* oStartPhantomWeaponMode)(uintptr_t* thisptr);
void __fastcall hkStartPhantomWeaponMode(uintptr_t* thisptr) {
	if (!bEnablePhantomWeapon)
		return oClearPhantomMode(thisptr);
	
	return oStartPhantomWeaponMode(thisptr);
}

bool __cdecl init([[maybe_unused]] const Version client_version)
{
    NtCurrentPeb()->BeingDebugged = FALSE;

    if (const auto module = pe::get_module()) {
        uintptr_t handle = module->handle();
        const auto sections = module->segments();
        const auto& s1 = std::find_if(sections.begin(), sections.end(), [](const IMAGE_SECTION_HEADER& x) {
            return x.Characteristics & IMAGE_SCN_CNT_CODE;
            });
        const auto data = s1->as_bytes();

		DetourTransactionBegin();
		DetourUpdateThread(NtCurrentThread());

		pugi::xml_parse_result loadResult = CfgDoc.load_file(docPath.c_str(), pugi::parse_default);

		// Failed to load XML document, abort like my mother should of done with me.
		if (!loadResult)
		{
			MessageBox(NULL, xorstr_(L"Failed to load extended_options.xml in Documents\\BnS"), xorstr_(L"Config Not Found"), MB_OK);
			return true;
		}
		auto sBinput = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("0F B6 47 18 48 8D 4C 24 30 89 03")));
		if (sBinput != data.end()) {
			uintptr_t aBinput = (uintptr_t)&sBinput[0] - 0x38;
			oBInputKey = module->rva_to<std::remove_pointer_t<decltype(oBInputKey)>>(aBinput - handle);
			DetourAttach(&(PVOID&)oBInputKey, &hkBInputKey);
		} else
			MessageBox(NULL, xorstr_(L"Failed to hook BInput"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		auto result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 C7 40 C8 FE FF FF FF 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 8B E9 41 0F B7 D8 4C 8B F2")));
		if (result != data.end()) {
			oPassiveEffectList_add = module->rva_to<std::remove_pointer_t<decltype(oPassiveEffectList_add)>>((uintptr_t)&result[0] - 0xD - handle);
			DataManager_Instance = (uintptr_t*)GetAddress((uintptr_t)&result[0] + 0x44, 3, 7); // 48 8B 05 90 F7 4B 03
			memcpy(&DataManager_EffectRecord, &result[0] + 0x4E, 4);
			//DataManager_Instance_Offset = memcpy(&walkBackOffset, &spfnc_gs[0] + 0x21, 4);

			DetourAttach(&(PVOID&)oPassiveEffectList_add, &hkPassiveEffectList_add);
		} else 
			MessageBox(NULL, xorstr_(L"Failed to hook PassiveEffect_add"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		auto sControlSet = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("F3 41 0F 10 08 0F 57 C0 0F 2F C8 72 0B 0F 28 C1")));
		if (sControlSet != data.end()) {
			oSystemFunc_SetOutFrustumParticleSpawnRate = module->rva_to<std::remove_pointer_t<decltype(oSystemFunc_SetOutFrustumParticleSpawnRate)>>((uintptr_t)&sControlSet[0] - handle);
			DetourAttach(&(PVOID&)oSystemFunc_SetOutFrustumParticleSpawnRate, &hkSystemFunc_SetOutFrustumParticleSpawnRate);
			EffectController = (FEngineControlSet*)(GetAddress((uintptr_t)&sControlSet[0] + 0x1E, 4, 8) - 0x24);
		}
		else
			MessageBox(NULL, xorstr_(L"Failed to find video options"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		auto sExecCmd = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B C3 0F 1F 44 00 00 48 FF C0 66 83 3C 41 00 75 ?? 48 85 C0  0F 84 1E 01 00 00")));
		if (sExecCmd != data.end()) {
			ExecuteConsoleCommandNoHistory = module->rva_to<std::remove_pointer_t<decltype(ExecuteConsoleCommandNoHistory)>>((uintptr_t)&sExecCmd[0] - 0x28 - handle);
		} else
			MessageBox(NULL, xorstr_(L"Failed to find ExecConsoleCommand, auto execution of console_cmds will not work"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		auto sAddNotif = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("45 33 DB 41 8D 42 ?? 3C 02 BB 05 00 00 00 41 0F 47 DB")));
		if (sAddNotif != data.end()) {
			AddInstantNotification = module->rva_to<std::remove_pointer_t<decltype(AddInstantNotification)>>((uintptr_t)&sAddNotif[0] - 0x68 - handle);
		}

		auto sPhantom = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B CB 33 FF 48 85 DB 48 0F 44 CF 80 B9 ?? ?? 00 00 02")));
		if (sPhantom != data.end()) {//BNSR.exe+4239B8D 

			oClearPhantomMode = module->rva_to<std::remove_pointer_t<decltype(oClearPhantomMode)>>(GetAddress((uintptr_t)&sPhantom[0] + 0x12B, 1, 5) - handle);
			oStartPhantomWeaponMode = module->rva_to<std::remove_pointer_t<decltype(oStartPhantomWeaponMode)>>(GetAddress((uintptr_t)&sPhantom[0] + 0x3C, 1, 5) - handle);
			oUpdatePhantomWeaponMode = module->rva_to<std::remove_pointer_t<decltype(oUpdatePhantomWeaponMode)>>((uintptr_t)&sPhantom[0] - 0xAD - handle);
			DetourAttach(&(PVOID&)oUpdatePhantomWeaponMode, &hkUpdatePhantomWeaponMode);
			DetourAttach(&(PVOID&)oStartPhantomWeaponMode, &hkStartPhantomWeaponMode);
		} else
			MessageBox(NULL, xorstr_(L"Failed to hook PhantomWeapon Functions"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 83 3D ?? ?? ?? ?? 00 49 8B F1 49 8B F8 8B EA 48 8B D9")));
		if (result != data.end()) {
			SignalInfo_Addr = GetAddress((uintptr_t)&result[0], 3, 8);
		}
		else
			MessageBox(NULL, xorstr_(L"Failed to find SignalInfo for Damage Font"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("48 8B 51 ?? ?? ?? ?? 74 10 33 C0 48 85 C9 48 0F 45 C2")));
		if (result != data.end()) {
			GetUiStateGame = module->rva_to<std::remove_pointer_t<decltype(GetUiStateGame)>>((uintptr_t)&result[0] - 0x18 - handle);
		}
		else
			MessageBox(NULL, xorstr_(L"Failed to find UiStateGame::Get()"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("45 33 ED 4C 8B 64 24 78 48 8B 8F ?? ?? ?? ?? 4C 8B 7C 24 60")));
		if (result != data.end()) {
			memcpy(&PassiveEffectList_Offset, &result[0] + 0xB, 4);
		}
		else
			MessageBox(NULL, xorstr_(L"Failed to find PassiveEffectList Offset"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		result = std::search(data.begin(), data.end(), pattern_searcher(xorstr_("66 89 54 24 10 48 89 4C 24 08 57 48 81 EC 10 02 00 00 48 C7 84 24 B8 00 00 00 FE FF FF FF")));
		if (result != data.end()) {
			BNSClient_GetWorld = module->rva_to<std::remove_pointer_t<decltype(BNSClient_GetWorld)>>(GetAddress(((uintptr_t)&result[0] + 0x38), 1, 5) - handle);
		}
		else 
			MessageBox(NULL, xorstr_(L"Failed to find BNSClient::GetWorld"), xorstr_(L"[ExtendedOptions] Search Error"), MB_OK);

		auto key = (CfgDoc.select_node(xorstr_("/config/options/option[@name='reloadKey']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("39"))); // Default is 6 key
		ReloadInput.Key = GetKeyCodeFromString(key);
		ReloadInput.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='reloadKey']")).node().attribute(xorstr_("bAlt")).as_bool(true));
		ReloadInput.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='reloadKey']")).node().attribute(xorstr_("bCtrl")).as_bool());
		ReloadInput.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='reloadKey']")).node().attribute(xorstr_("bShift")).as_bool());

		auto profile_1 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_1']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("31"))); // Default is 6 key
		Profile_1.Key = GetKeyCodeFromString(profile_1);
		Profile_1.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_1']")).node().attribute(xorstr_("bAlt")).as_bool(true));
		Profile_1.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_1']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
		Profile_1.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_1']")).node().attribute(xorstr_("bShift")).as_bool());

		auto profile_2 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_2']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("32"))); // Default is 6 key
		Profile_2.Key = GetKeyCodeFromString(profile_2);
		Profile_2.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_2']")).node().attribute(xorstr_("bAlt")).as_bool(true));
		Profile_2.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_2']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
		Profile_2.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_2']")).node().attribute(xorstr_("bShift")).as_bool());

		auto profile_3 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_3']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("33"))); // Default is 6 key
		Profile_3.Key = GetKeyCodeFromString(profile_3);
		Profile_3.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_3']")).node().attribute(xorstr_("bAlt")).as_bool(true));
		Profile_3.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_3']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
		Profile_3.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_3']")).node().attribute(xorstr_("bShift")).as_bool());

		auto profile_4 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_4']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("33"))); // Default is 6 key
		Profile_4.Key = GetKeyCodeFromString(profile_4);
		Profile_4.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_4']")).node().attribute(xorstr_("bAlt")).as_bool(true));
		Profile_4.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_4']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
		Profile_4.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_4']")).node().attribute(xorstr_("bShift")).as_bool());

		auto profile_5 = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_5']")).node().attribute(xorstr_("keyCode")).as_string(xorstr_("33"))); // Default is 6 key
		Profile_5.Key = GetKeyCodeFromString(profile_5);
		Profile_5.bAltPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_5']")).node().attribute(xorstr_("bAlt")).as_bool(true));
		Profile_5.bCtrlPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_5']")).node().attribute(xorstr_("bCtrl")).as_bool(true));
		Profile_5.bShiftPressed = (CfgDoc.select_node(xorstr_("/config/options/option[@name='profile_5']")).node().attribute(xorstr_("bShift")).as_bool());

		DetourTransactionCommit();
    }
    return true;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (docPath.empty())
			docPath = documents_path() / xorstr_(L"BnS\\extended_options.xml");

		DisableThreadLibraryCalls(hInstance);
	}

	return TRUE;
}

extern "C" __declspec(dllexport) PluginInfo GPluginInfo = {
  .hide_from_peb = true,
  .erase_pe_header = true,
  .init = init,
  .priority = 1,
  .target_apps = L"BNSR.exe"
};