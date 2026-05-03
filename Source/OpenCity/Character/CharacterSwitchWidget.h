#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterSwitchWidget.generated.h"

class UCharacterDataAsset;
class UBorder;
class UHorizontalBox;
class USizeBox;
class UTextBlock;

/**
 * Full-screen UMG overlay displayed when the player presses Tab to switch roles.
 * Slides up from the bottom as a row of icon cards — one per roster entry.
 * Fully constructed in C++; no Blueprint asset required.
 */
UCLASS()
class OPENCITY_API UCharacterSwitchWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Provide the ordered role list. Rebuilds the card row.
    void SetRoster(const TArray<TObjectPtr<UCharacterDataAsset>> &InRoster);

    // Refreshes card highlight. Call whenever the selection index changes.
    void SetSelectedIndex(int32 Index);

protected:
    virtual bool Initialize() override;

private:
    void RebuildCards();
    FLinearColor RoleColorForIndex(int32 Index) const;

    // Populated in Initialize(); cards are inserted here by RebuildCards().
    UPROPERTY()
    TObjectPtr<UHorizontalBox> CardRow;

    // One border per roster entry — recoloured / scaled by SetSelectedIndex.
    UPROPERTY()
    TArray<TObjectPtr<UBorder>> CardBorders;

    TArray<TObjectPtr<UCharacterDataAsset>> Roster;
    int32 SelectedIndex = 0;
};
