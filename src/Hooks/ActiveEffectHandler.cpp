#include "Hooks/ActiveEffectHandler.h"

#include "Hooks/PoiseAV.h"
#include "Storage/Settings.h"

float ActiveEffectHandler::CalculateEffectMultiplier(RE::ActorValue a_actorValue, bool a_detrimental)
{
	auto        settings = Settings::GetSingleton();
	std::string sEffectType = a_detrimental ? "Damage" : "Recovery";
	std::string baseAVString = std::string(magic_enum::enum_name(a_actorValue));
	if (baseAVString.size() != 0){
		baseAVString = baseAVString.substr(1);
		auto actorValue = settings->JSONSettings["Magic Effects"]["Actor Values"][sEffectType][baseAVString];
		
		if (actorValue != nullptr)
			return static_cast<float>(actorValue);
	}

	return 0;
}

void ActiveEffectHandler::ProcessValueModifier(RE::Actor* a_target, RE::ActorValue a_actorValue, float a_magnitudeDelta, RE::Actor* a_aggressor)
{
	auto poiseAV = PoiseAV::GetSingleton();
	auto settings = Settings::GetSingleton();

	if (a_target != a_aggressor && (a_aggressor || a_magnitudeDelta < 0) && poiseAV->CanDamageActor(a_target) && a_magnitudeDelta != 0) {
		float poiseDamage = CalculateEffectMultiplier(a_actorValue, a_magnitudeDelta > 0) * a_magnitudeDelta;

		if (a_aggressor) {
			float baseMult = 1.0f;
			PoiseAV::ApplyPerkEntryPoint(34, a_aggressor->As<RE::Character>(), a_target->As<RE::Character>(), &baseMult);
			PoiseAV::ApplyPerkEntryPoint(33, a_target->As<RE::Character>(), a_aggressor->As<RE::Character>(), &baseMult);
			poiseDamage *= baseMult;
			if (poiseDamage > 0)
			{
				poiseDamage *= settings->GetDamageMultiplier(a_aggressor, a_target);
				if (a_target != a_aggressor) {
					if (a_target->IsPlayerRef())
						poiseDamage *= settings->Damage.ToPCMult;
					else
						poiseDamage *= settings->Damage.ToNPCMult;
				}
			}
		}

		poiseAV->DamageAndCheckPoise(a_target, a_aggressor, poiseDamage);
	}
}


