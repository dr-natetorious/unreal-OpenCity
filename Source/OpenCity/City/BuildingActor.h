#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/BuildingPlacer.h"
#include "BuildingActor.generated.h"

class UStaticMeshComponent;

// A single building rendered as a scaled unit cube.
// Assign a 1m×1m×1m static mesh in Blueprint; SetSpec() scales it to the desired footprint.
UCLASS()
class OPENCITY_API ABuildingActor : public AActor
{
    GENERATED_BODY()

public:
    ABuildingActor();

    // Apply a building spec — sets location, scale, and collision.
    void SetSpec(const FBuildingSpec& Spec);

    const FBuildingSpec& GetSpec() const { return CachedSpec; }

private:
    UPROPERTY(VisibleAnywhere, Category="OpenCity|City")
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    FBuildingSpec CachedSpec;
};
