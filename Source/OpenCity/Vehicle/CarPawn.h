#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "Core/VehicleParams.h"
#include "CarPawn.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class USphereComponent;
class UInputMappingContext;
class UInputAction;
class AOpenCityCharacter;

UCLASS()
class OPENCITY_API ACarPawn : public APawn
{
    GENERATED_BODY()

public:
    ACarPawn();

    /** Called by the character's interact handler to board this car. */
    void EnterCar(AOpenCityCharacter* Driver);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    FCarParams CarParams;

protected:
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void NotifyControllerChanged() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    void HandleThrottle(const FInputActionValue& Value);
    void HandleSteering(const FInputActionValue& Value);
    void HandleInteract(const FInputActionValue& Value);

    UFUNCTION()
    void OnProximityBegin(UPrimitiveComponent* OverlappedComp, AActor* Other,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnProximityEnd(UPrimitiveComponent* OverlappedComp, AActor* Other,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // ── Geometry ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, Category="OpenCity|Vehicle")
    TObjectPtr<UBoxComponent> CollisionBox;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Vehicle")
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Vehicle")
    TObjectPtr<UStaticMeshComponent> WheelFL;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Vehicle")
    TObjectPtr<UStaticMeshComponent> WheelFR;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Vehicle")
    TObjectPtr<UStaticMeshComponent> WheelRL;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Vehicle")
    TObjectPtr<UStaticMeshComponent> WheelRR;

    // ── Interaction ──────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, Category="OpenCity|Vehicle")
    TObjectPtr<USphereComponent> ProximitySphere;

    // Character standing nearby (set by overlap events, cleared on enter/exit)
    TObjectPtr<AOpenCityCharacter> NearbyCharacter;

    // Character currently driving (nullptr when unoccupied)
    TObjectPtr<AOpenCityCharacter> OccupyingDriver;

    // ── Camera ───────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, Category="OpenCity|Camera")
    TObjectPtr<USpringArmComponent> CameraArm;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    // ── Input ────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputMappingContext> VehicleMappingContext;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> ThrottleAction;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> SteeringAction;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> InteractAction;

    // ── State ────────────────────────────────────────────────────────────────
    float CurrentThrottle = 0.f;
    float CurrentSteering = 0.f;

    // Wheel spin accumulator (degrees)
    float WheelAngle = 0.f;

    static constexpr float MaxSpeedCmS   = 1500.f;  // 54 km/h
    static constexpr float Acceleration  = 800.f;   // cm/s² input scale
    static constexpr float TurnRateDegS  = 90.f;    // degrees/s at full steer
    static constexpr float WheelRadiusCm = 35.f;
};
