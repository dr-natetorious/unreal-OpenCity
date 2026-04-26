#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "OpenCityCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMeshComponent;
class ACarPawn;

UCLASS()
class OPENCITY_API AOpenCityCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AOpenCityCharacter();

    /** Called by ACarPawn's proximity sphere — tells us a car is enterable. */
    void SetNearbyCar(ACarPawn* Car) { NearbyCar = Car; }

protected:
    virtual void NotifyControllerChanged() override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    virtual void Tick(float DeltaSeconds) override;

private:
    void HandleMove(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);
    void HandleInteract(const FInputActionValue& Value);

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Mesh")
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    UPROPERTY(VisibleAnywhere, Category="OpenCity|Mesh")
    TObjectPtr<UStaticMeshComponent> HeadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OpenCity|Camera", meta=(AllowPrivateAccess="true"))
    TObjectPtr<USpringArmComponent> CameraArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="OpenCity|Camera", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputMappingContext> FootMappingContext;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, Category="OpenCity|Input")
    TObjectPtr<UInputAction> InteractAction;

    // Nearest car within proximity — set by ACarPawn overlap events
    TObjectPtr<ACarPawn> NearbyCar;
};
