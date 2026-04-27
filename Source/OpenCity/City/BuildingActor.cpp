#include "City/BuildingActor.h"
#include "Components/StaticMeshComponent.h"

ABuildingActor::ABuildingActor()
{
    PrimaryActorTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
        TEXT("/Engine/BasicShapes/Cube"));

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    SetRootComponent(BodyMesh);
    BodyMesh->SetCollisionProfileName(TEXT("BlockAll"));
    if (CubeAsset.Succeeded()) BodyMesh->SetStaticMesh(CubeAsset.Object);
}

void ABuildingActor::SetSpec(const FBuildingSpec& Spec)
{
    CachedSpec = Spec;

    // Pivot is at the bottom center. Raise by half-height so the mesh sits on the ground.
    SetActorLocation(FVector(Spec.WorldXCm, Spec.WorldYCm, Spec.HeightCm * 0.5f));

    // Scale a 100cm unit cube to the desired dimensions.
    BodyMesh->SetRelativeScale3D(FVector(
        Spec.WidthCm  / 100.f,
        Spec.DepthCm  / 100.f,
        Spec.HeightCm / 100.f
    ));
}
