#pragma once
#include "Engine/Engine.h"
#include "Engine/World.h"

// Returns the active test world regardless of context:
//   - EWorldType::PIE      — editor with Play active
//   - EWorldType::Game     — launched with -Game
//   - EWorldType::Editor   — headless editor commandlet
// Returns nullptr only when no world context exists at all.
inline UWorld* GetTestWorld()
{
    if (!GEngine) return nullptr;

    UWorld* Fallback = nullptr;
    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if (Ctx.WorldType == EWorldType::PIE || Ctx.WorldType == EWorldType::Game)
            return Ctx.World();
        if (Ctx.World())
            Fallback = Ctx.World();
    }
    return Fallback;
}
