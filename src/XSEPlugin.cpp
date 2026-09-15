#include "ActorValues/AVManager.h"
#include "AnimEventHandler.h"
#include "EldenParry.h"
#include "Events/Events.h"
#include "Hooks.h"
#include "Hooks/Hooks.h"
#include "Hooks/PoiseAV.h"
#include "Settings.h"
#include "Storage/ActorCache.h"
#include "Storage/Serialization.h"
#include "Storage/Settings.h"
#include "UI/PoiseAVHUD.h"
#include "Utils.hpp"

void MessageHandler(SKSE::MessagingInterface::Message *a_msg)
{
    switch (a_msg->type)
    {
    case SKSE::MessagingInterface::kDataLoaded:
    {
        auto poiseAV = PoiseAV::GetSingleton();
        poiseAV->RetrieveFullBodyStaggerFaction();
        auto settings = Settings::GetSingleton();
        settings->LoadSettings();
        EldenParry::GetSingleton()->init();
        animEventHandler::Register(true, EldenSettings::bEnableNPCParry);
        break;
    }
    case SKSE::MessagingInterface::kPostLoad:
    {
        auto poiseAVHUD = PoiseAVHUD::GetSingleton();
        poiseAVHUD->trueHUDInterface = reinterpret_cast<TRUEHUD_API::IVTrueHUD3 *>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V3));
        if (poiseAVHUD->trueHUDInterface)
        {
            logger::info("Obtained TrueHUD API");
        }
        else
        {
            logger::warn("Failed to obtain TrueHUD API");
        }
        break;
    }
    case SKSE::MessagingInterface::kPreLoadGame:
    {
        auto settings = Settings::GetSingleton();
        settings->LoadSettings();
        ActorCache::GetSingleton()->Revert();
        break;
    }
    case SKSE::MessagingInterface::kNewGame:
    {
        auto settings = Settings::GetSingleton();
        settings->LoadSettings();
        ActorCache::GetSingleton()->Revert();
        break;
    }
    default:
        break;
    }
}

void Init() {
    // hooks::OnMeleeHitHook::install_pluginListener();
    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener("SKSE", MessageHandler);
}

void Load([[maybe_unused]] bool VR)
{
    // hooks::OnMeleeHitHook::install_protected();
    // hooks::InstallCombat(VR);
    // hooks::DrinkPotionHook::Install();
}

void onSKSEInit() {
    // const auto papyrus = SKSE::GetPapyrusInterface();
    // papyrus->Register(hooks::OnMeleeHitHook::BindPapyrusFunctions);

    auto poiseAV = PoiseAV::GetSingleton();
    auto avManager = AVManager::GetSingleton();
    avManager->RegisterActorValue(PoiseAV::g_avName, poiseAV);

    auto messaging = SKSE::GetMessagingInterface();
    messaging->RegisterListener("SKSE", MessageHandler);

    auto serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(Serialization::kUniqueID);
    serialization->SetSaveCallback(Serialization::SaveCallback);
    serialization->SetLoadCallback(Serialization::LoadCallback);
    serialization->SetRevertCallback(Serialization::RevertCallback);

    Hooks::Install();
    Events::Register();
    ActorCache::RegisterEvents();
    EldenSettings::readSettings();
    EldenHooks::install();
    Milf::GetSingleton()->Load();
}

void PreLoad() {
    
}