#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PedestrianCharacter.generated.h"

class UStaticMeshComponent;

// Dummy pedestrian NPC. Patrols A↔B on a sidewalk segment.
// Spawned and destroyed by ACityBlockActor alongside buildings.
UCLASS()
class OPENCITY_API APedestrianCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    APedestrianCharacter();

    // Set the patrol route and walk speed. Teleports to WaypointA immediately.
    void InitPatrol(FVector A, FVector B, float SpeedCmS);

    virtual void Tick(float DeltaSeconds) override;

#if WITH_AUTOMATION_TESTS
    FVector Test_GetCurrentWaypoint() const;
#endif

private:
    FVector WaypointA = FVector::ZeroVector;
    FVector WaypointB = FVector::ZeroVector;
    int32   CurrentTarget = 1;   // 0=A, 1=B; starts at A, walks toward B
    float   WaypointReachRadiusCm = 60.f;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Mesh")
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Mesh")
    TObjectPtr<UStaticMeshComponent> HeadMesh;
};
