#pragma once
#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "InputActionValue.h"
#include "Core/VehicleParams.h"
#include "CarPawn.generated.h"

class UChaosWheeledVehicleMovementComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMeshComponent;

UCLASS()
class OPENCITY_API ACarPawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    ACarPawn();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    FCarParams CarParams;

protected:
    virtual void NotifyControllerChanged() override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void Tick(float DeltaSeconds) override;

private:
    void HandleThrottle(const FInputActionValue& Value);
    void HandleSteering(const FInputActionValue& Value);
    void HandleBrake(const FInputActionValue& Value);
    void HandleHandbrake(const FInputActionValue& Value);

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Camera")
    TObjectPtr<USpringArmComponent> CameraArm;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputMappingContext> VehicleMappingContext;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> ThrottleAction;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> SteeringAction;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> BrakeAction;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> HandbrakeAction;

    TObjectPtr<UChaosWheeledVehicleMovementComponent> VehicleMovement;
};
