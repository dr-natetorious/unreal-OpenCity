#include "Character/PedestrianCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

APedestrianCharacter::APedestrianCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Slightly narrower capsule than the player (20cm radius, 90cm half-height → 1.8m tall).
    GetCapsuleComponent()->InitCapsuleSize(20.f, 90.f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;

    GetCharacterMovement()->bOrientRotationToMovement  = true;
    GetCharacterMovement()->RotationRate               = FRotator(0.f, 360.f, 0.f);
    GetCharacterMovement()->MaxWalkSpeed               = 140.f;
    GetCharacterMovement()->MinAnalogWalkSpeed         = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(
        TEXT("/Engine/BasicShapes/Cylinder"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(
        TEXT("/Engine/BasicShapes/Sphere"));

    // Body: cylinder scaled to 40cm wide × 140cm tall.
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(GetCapsuleComponent());
    BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -15.f));
    BodyMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 1.4f));
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (CylinderAsset.Succeeded())
        BodyMesh->SetStaticMesh(CylinderAsset.Object);

    // Head: sphere scaled to 30cm diameter.
    HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
    HeadMesh->SetupAttachment(GetCapsuleComponent());
    HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 75.f));
    HeadMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
    HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (SphereAsset.Succeeded())
        HeadMesh->SetStaticMesh(SphereAsset.Object);
}

void APedestrianCharacter::InitPatrol(FVector A, FVector B, float SpeedCmS)
{
    WaypointA = A;
    WaypointB = B;
    CurrentTarget = 1;   // start at A, walk toward B
    GetCharacterMovement()->MaxWalkSpeed = SpeedCmS;
    SetActorLocation(A);
}

void APedestrianCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const FVector Target = (CurrentTarget == 0) ? WaypointA : WaypointB;
    const FVector Delta  = Target - GetActorLocation();
    const float   Dist   = Delta.Size2D();

    if (Dist < WaypointReachRadiusCm)
    {
        CurrentTarget = 1 - CurrentTarget;
    }
    else
    {
        AddMovementInput(Delta.GetSafeNormal2D());
    }
}

#if WITH_AUTOMATION_TESTS
FVector APedestrianCharacter::Test_GetCurrentWaypoint() const
{
    return (CurrentTarget == 0) ? WaypointA : WaypointB;
}
#endif
