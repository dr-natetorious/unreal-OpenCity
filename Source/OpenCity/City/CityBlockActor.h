#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/CityGrid.h"
#include "Core/BuildingPlacer.h"
#include "Core/PedestrianParams.h"
#include "CityBlockActor.generated.h"

class ABuildingActor;
class APedestrianCharacter;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|NPC")
    FPedestrianParams PedestrianParams;

    // (Re)generate buildings and pedestrians. Safe to call from editor or at runtime.
    UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenCity|City")
    void GenerateBuildings();

    void ClearBuildings();

    const TArray<TObjectPtr<ABuildingActor>>&    GetBuildings()   const { return SpawnedBuildings; }
    const TArray<TObjectPtr<APedestrianCharacter>>& GetPedestrians() const { return SpawnedPedestrians; }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(VisibleAnywhere, Category="OpenCity|City")
    TArray<TObjectPtr<ABuildingActor>> SpawnedBuildings;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|NPC")
    TArray<TObjectPtr<APedestrianCharacter>> SpawnedPedestrians;

    void SpawnPedestrians();
    void ClearPedestrians();
};
