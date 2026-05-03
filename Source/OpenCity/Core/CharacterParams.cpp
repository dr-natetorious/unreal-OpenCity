#include "Core/CharacterParams.h"
#include "NativeGameplayTags.h"

// ── Action capability tags ──────────────────────────────────────────────────
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanArrest, "Character.Capability.CanArrest")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanFightFire, "Character.Capability.CanFightFire")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanHeal, "Character.Capability.CanHeal")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanFly, "Character.Capability.CanFly")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanRepair, "Character.Capability.CanRepair")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanRescue, "Character.Capability.CanRescue")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanDriveEmergency, "Character.Capability.CanDriveEmergency")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanHack, "Character.Capability.CanHack")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanInvestigate, "Character.Capability.CanInvestigate")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_CanParachute, "Character.Capability.CanParachute")

// ── NPC state tags ───────────────────────────────────────────────────────────
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_IsArrestable, "Character.Capability.IsArrestable")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_IsOnFire, "Character.Capability.IsOnFire")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_NeedsRescue, "Character.Capability.NeedsRescue")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_IsInjured, "Character.Capability.IsInjured")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_IsSuspect, "Character.Capability.IsSuspect")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_IsWitness, "Character.Capability.IsWitness")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_IsVIP, "Character.Capability.IsVIP")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_IsRobot, "Character.Capability.IsRobot")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_HasBadge, "Character.Capability.HasBadge")
UE_DEFINE_GAMEPLAY_TAG(TAG_Capability_HasTools, "Character.Capability.HasTools")

bool FAppearanceParams::AreValid() const
{
    // Any FName that is set (non-default NAME_None) or explicitly "None"/"Default" is valid.
    // The default field values ("None", "Default") always satisfy this.
    static const FName NoneName(TEXT("None"));
    static const FName DefaultName(TEXT("Default"));
    const bool HatOk = HatStyle == NoneName || !HatStyle.IsNone();
    const bool AccessoryOk = AccessoryStyle == NoneName || !AccessoryStyle.IsNone();
    const bool FaceOk = FaceStyle == DefaultName || !FaceStyle.IsNone();
    const bool SocketOk = !HatSocket.IsNone() && !LeftHandAccessorySocket.IsNone() && !RightHandAccessorySocket.IsNone();
    return HatOk && AccessoryOk && FaceOk && SocketOk;
}

bool FCapabilitySet::AreValid() const
{
    // Empty is valid — a plain civilian has no special capabilities.
    TSet<FGameplayTag> Seen;
    for (const FGameplayTag &Tag : Capabilities)
    {
        if (Seen.Contains(Tag))
            return false;
        Seen.Add(Tag);
    }
    return true;
}

bool FCapabilitySet::HasCapability(const FGameplayTag &Tag) const
{
    return Capabilities.Contains(Tag);
}

FLinearColor CharacterAppearancePolicy::ResolveShirtColor(const FAppearanceParams &Appearance,
                                                          const FCapabilitySet &Capabilities)
{
    // Firefighters need strongest visual priority in mixed emergency scenes.
    if (Capabilities.HasCapability(TAG_Capability_CanFightFire.GetTag()))
        return FLinearColor(0.85f, 0.12f, 0.12f, 1.f);

    if (Capabilities.HasCapability(TAG_Capability_CanArrest.GetTag()))
        return FLinearColor(0.12f, 0.27f, 0.86f, 1.f);

    if (Capabilities.HasCapability(TAG_Capability_CanHeal.GetTag()))
        return FLinearColor(0.1f, 0.72f, 0.2f, 1.f);

    return Appearance.ShirtColor;
}
