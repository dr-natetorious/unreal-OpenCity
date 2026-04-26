#pragma once
#include "CoreMinimal.h"
#include "VehicleParams.generated.h"

// Physical and visual parameters for a wheeled vehicle.
// All distances in meters; convert to cm (*100) when passing to UE components.
USTRUCT(BlueprintType)
struct OPENCITY_API FCarParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float MassKg = 1500.f;

    // Front-to-rear axle distance, meters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float WheelbaseM = 2.7f;

    // Left-to-right wheel center distance, meters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float TrackWidthM = 1.52f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float WheelRadiusM = 0.33f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float WheelWidthM = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float MaxSteerDeg = 35.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float MaxSpeedKmH = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float MaxEngineTorqueNm = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float MaxBrakeTorqueNm = 3000.f;

    // Visual body extents, meters. Pivot at bottom center.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float BodyLengthM = 4.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float BodyWidthM = 1.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float BodyHeightM = 1.4f;

    // Height of the bottom of the body above the ground plane, meters.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenCity|Vehicle")
    float BodyZOffsetM = 0.35f;

    bool AreValid() const
    {
        return MassKg > 0.f
            && WheelbaseM > 0.f
            && TrackWidthM > 0.f
            && WheelRadiusM > 0.f
            && MaxSpeedKmH > 0.f
            && MaxEngineTorqueNm > 0.f;
    }

    // Wheel positions relative to actor root at ground level, in cm.
    // Order: front-left, front-right, rear-left, rear-right.
    TArray<FVector> GetWheelPositionsCm() const
    {
        const float HWB = WheelbaseM  * 0.5f * 100.f;
        const float HTW = TrackWidthM * 0.5f * 100.f;
        const float WZ  = WheelRadiusM * 100.f;
        return {
            FVector( HWB,  HTW, WZ),
            FVector( HWB, -HTW, WZ),
            FVector(-HWB,  HTW, WZ),
            FVector(-HWB, -HTW, WZ),
        };
    }
};
