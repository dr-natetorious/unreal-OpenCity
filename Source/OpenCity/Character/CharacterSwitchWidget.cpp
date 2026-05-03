#include "Character/CharacterSwitchWidget.h"
#include "Character/CharacterDataAsset.h"
#include "Core/CharacterParams.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Misc/Paths.h"

// ── FontAwesome 6 Free Solid codepoints (BMP Private Use Area) ───────────────
// Verified in fa-solid-900.ttf using the ligatures table:
//   shield-halved  F3ED   → Officer
//   fire           F06D   → Firefighter
//   kit-medical    F479   → EMT  (was "medkit" F0FA in FA4/5; FA6 remapped to F479)
//   user           F007   → fallback
namespace FAGlyph
{
    static constexpr TCHAR Officer[] = {(TCHAR)0xF3ED, TEXT('\0')};
    static constexpr TCHAR Firefighter[] = {(TCHAR)0xF06D, TEXT('\0')};
    static constexpr TCHAR EMT[] = {(TCHAR)0xF479, TEXT('\0')};
    static constexpr TCHAR Default[] = {(TCHAR)0xF007, TEXT('\0')};

    static const TCHAR *ForIndex(int32 i)
    {
        switch (i)
        {
        case 0:
            return Officer;
        case 1:
            return Firefighter;
        case 2:
            return EMT;
        default:
            return Default;
        }
    }
}

static FSlateFontInfo MakeIconFont(int32 Size)
{
    const FString FontPath = FPaths::ProjectContentDir() / TEXT("UI/Fonts/fa-solid-900.ttf");
    return FSlateFontInfo(FontPath, Size);
}

bool UCharacterSwitchWidget::Initialize()
{
    if (!Super::Initialize())
        return false;

    // ── Root: full-viewport canvas ────────────────────────────────────────
    UCanvasPanel *Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
    WidgetTree->RootWidget = Root;

    // ── Pill container anchored to bottom-center ──────────────────────────
    UBorder *Pill = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Pill"));
    {
        FSlateBrush PillBrush;
        PillBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
        PillBrush.OutlineSettings.CornerRadii = FVector4(12.f, 12.f, 12.f, 12.f);
        PillBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
        PillBrush.TintColor = FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.78f));
        Pill->SetBrush(PillBrush);
        Pill->SetPadding(FMargin(28.f, 18.f, 28.f, 20.f));
    }
    UCanvasPanelSlot *PillSlot = Root->AddChildToCanvas(Pill);
    PillSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
    PillSlot->SetAlignment(FVector2D(0.5f, 1.0f));
    PillSlot->SetPosition(FVector2D(0.f, -64.f));
    PillSlot->SetAutoSize(true);

    // ── Outer vertical stack (title + cards + hint) ───────────────────────
    UVerticalBox *Stack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Stack"));
    Pill->SetContent(Stack);

    // Title
    UTextBlock *Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
    Title->SetText(FText::FromString(TEXT("SWITCH CHARACTER")));
    Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f, 1.f)));
    {
        FSlateFontInfo F = Title->GetFont();
        F.Size = 13;
        F.TypefaceFontName = TEXT("Bold");
        Title->SetFont(F);
    }
    Title->SetJustification(ETextJustify::Center);
    {
        UVerticalBoxSlot *S = Stack->AddChildToVerticalBox(Title);
        S->SetHorizontalAlignment(HAlign_Center);
        S->SetPadding(FMargin(0.f, 0.f, 0.f, 14.f));
    }

    // Card row (populated later by SetRoster → RebuildCards)
    CardRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CardRow"));
    {
        UVerticalBoxSlot *S = Stack->AddChildToVerticalBox(CardRow);
        S->SetHorizontalAlignment(HAlign_Center);
        S->SetPadding(FMargin(0.f, 0.f, 0.f, 0.f));
    }

    // Hint bar
    UTextBlock *Hint = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Hint"));
    Hint->SetText(FText::FromString(TEXT("\u2190 \u2192  Navigate      Enter  Confirm      Esc  Cancel")));
    Hint->SetColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f, 1.f)));
    {
        FSlateFontInfo F = Hint->GetFont();
        F.Size = 10;
        Hint->SetFont(F);
    }
    Hint->SetJustification(ETextJustify::Center);
    {
        UVerticalBoxSlot *S = Stack->AddChildToVerticalBox(Hint);
        S->SetHorizontalAlignment(HAlign_Center);
        S->SetPadding(FMargin(0.f, 14.f, 0.f, 0.f));
    }

    return true;
}

void UCharacterSwitchWidget::SetRoster(const TArray<TObjectPtr<UCharacterDataAsset>> &InRoster)
{
    Roster = InRoster;
    RebuildCards();
}

void UCharacterSwitchWidget::RebuildCards()
{
    if (!CardRow)
        return;

    CardRow->ClearChildren();
    CardBorders.Reset();

    const FSlateFontInfo IconFont = MakeIconFont(44);

    for (int32 i = 0; i < Roster.Num(); ++i)
    {
        const UCharacterDataAsset *Entry = Roster[i];
        const FString Name = (Entry && !Entry->CharacterName.IsEmpty())
                                 ? Entry->CharacterName.ToString()
                                 : FString::Printf(TEXT("Role %d"), i + 1);

        const FLinearColor RoleColor = RoleColorForIndex(i);

        // ── Card border (rounded, dark background) ────────────────────────
        UBorder *Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
        {
            FSlateBrush B;
            B.DrawAs = ESlateBrushDrawType::RoundedBox;
            B.OutlineSettings.CornerRadii = FVector4(8.f, 8.f, 8.f, 8.f);
            B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
            B.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.92f));
            Card->SetBrush(B);
            Card->SetPadding(FMargin(18.f, 14.f));
        }

        // ── Card inner vertical stack (icon + name) ───────────────────────
        UVerticalBox *Inner = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

        // Icon glyph
        UTextBlock *IconText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        IconText->SetFont(IconFont);
        IconText->SetText(FText::FromString(FString(FAGlyph::ForIndex(i))));
        IconText->SetColorAndOpacity(FSlateColor(RoleColor));
        IconText->SetJustification(ETextJustify::Center);
        {
            UVerticalBoxSlot *S = Inner->AddChildToVerticalBox(IconText);
            S->SetHorizontalAlignment(HAlign_Center);
            S->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
        }

        // Role name
        UTextBlock *NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
        {
            FSlateFontInfo F = NameText->GetFont();
            F.Size = 11;
            NameText->SetFont(F);
        }
        NameText->SetText(FText::FromString(Name));
        NameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.f)));
        NameText->SetJustification(ETextJustify::Center);
        {
            UVerticalBoxSlot *S = Inner->AddChildToVerticalBox(NameText);
            S->SetHorizontalAlignment(HAlign_Center);
        }

        Card->SetContent(Inner);

        // ── Fixed-width wrapper so cards stay equal width ─────────────────
        USizeBox *Sizer = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
        Sizer->SetMinDesiredWidth(110.f);
        Sizer->SetMinDesiredHeight(130.f);
        Sizer->SetContent(Card);

        UHorizontalBoxSlot *HSlot = CardRow->AddChildToHorizontalBox(Sizer);
        HSlot->SetPadding(FMargin(8.f, 0.f));
        HSlot->SetHorizontalAlignment(HAlign_Fill);
        HSlot->SetVerticalAlignment(VAlign_Center);

        CardBorders.Add(Card);
    }

    SetSelectedIndex(SelectedIndex);
}

void UCharacterSwitchWidget::SetSelectedIndex(int32 Index)
{
    SelectedIndex = FMath::Clamp(Index, 0, FMath::Max(0, Roster.Num() - 1));

    for (int32 i = 0; i < CardBorders.Num(); ++i)
    {
        if (!CardBorders[i])
            continue;

        const bool bSelected = (i == SelectedIndex);
        const FLinearColor RoleColor = RoleColorForIndex(i);

        CardBorders[i]->SetRenderOpacity(bSelected ? 1.0f : 0.42f);
        CardBorders[i]->SetRenderScale(FVector2D(bSelected ? 1.08f : 0.93f));

        FSlateBrush B;
        B.DrawAs = ESlateBrushDrawType::RoundedBox;
        B.OutlineSettings.CornerRadii = FVector4(8.f, 8.f, 8.f, 8.f);
        B.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
        if (bSelected)
        {
            // Tinted by the role's colour at 35% blend, plus a bright outline tint
            B.TintColor = FSlateColor(FLinearColor(
                0.08f + RoleColor.R * 0.28f,
                0.08f + RoleColor.G * 0.28f,
                0.08f + RoleColor.B * 0.28f,
                0.96f));
            B.OutlineSettings.Color = FSlateColor(RoleColor);
            B.OutlineSettings.Width = 2.f;
        }
        else
        {
            B.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.92f));
        }
        CardBorders[i]->SetBrush(B);
    }
}

FLinearColor UCharacterSwitchWidget::RoleColorForIndex(int32 Index) const
{
    if (Roster.IsValidIndex(Index) && Roster[Index])
    {
        return CharacterAppearancePolicy::ResolveShirtColor(
            Roster[Index]->Appearance,
            Roster[Index]->Capabilities);
    }
    return FLinearColor::White;
}
