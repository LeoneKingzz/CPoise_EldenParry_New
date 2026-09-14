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

constexpr auto kTrampolineSize = 14;

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

void InitializeLog()
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto       sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
	const auto level = spdlog::level::trace;
#else
	const auto level = spdlog::level::info;
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);
}

SKSEPluginInfo(
    .Version = Plugin::VERSION,
    .Name = Plugin::NAME.data(),
    .Author = "LeoneKingzz",
    .StructCompatibility = SKSE::StructCompatibility::Independent,
    .RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary
);

SKSEPluginLoad(const SKSE::LoadInterface* a_skse) {

    InitializeLog();
    logger::info("Loaded plugin");
    
    SKSE::Init(a_skse, false);
    // SKSE::Init(a_skse, {1 << 5});
    SKSE::AllocTrampoline(kTrampolineSize);
    // SKSE::AllocTrampoline(1 << 5);
    PreLoad();
    onSKSEInit();
    Init();
    const auto ver = a_skse->RuntimeVersion();
    if (ver == SKSE::RUNTIME_LATEST_VR) {
        Load(true);
    } else {
        Load(false);
    }

    return true;
}
