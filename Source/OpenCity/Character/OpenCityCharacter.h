#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "OpenCityCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class OPENCITY_API AOpenCityCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AOpenCityCharacter();

protected:
    virtual void NotifyControllerChanged() override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;

private:
    void HandleMove(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);

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
};
