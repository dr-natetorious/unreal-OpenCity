#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/CityGrid.h"
#include "Core/BuildingPlacer.h"
#include "CityBlockActor.generated.h"

class ABuildingActor;

// Spawns and owns all buildings for one city cell.
// Place in the level and set CellX/CellY; call GenerateBuildings() or let BeginPlay do it.
UCLASS()
class OPENCITY_API ACityBlockActor : public AActor
{
    GENERATED_BODY()

public:
    ACityBlockActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    int32 CellX = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    int32 CellY = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    FCityGridParams GridParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    FBuildingPlacerParams BuildingParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|City")
    int32 Seed = 42;

    // (Re)generate buildings. Safe to call from editor or at runtime.
    UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenCity|City")
    void GenerateBuildings();

    void ClearBuildings();

    const TArray<TObjectPtr<ABuildingActor>>& GetBuildings() const { return SpawnedBuildings; }

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="OpenCity|City")
    TArray<TObjectPtr<ABuildingActor>> SpawnedBuildings;
};
