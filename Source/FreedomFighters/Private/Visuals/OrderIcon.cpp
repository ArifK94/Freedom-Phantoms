#include "Visuals/OrderIcon.h"
#include "Characters/BaseCharacter.h"

#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"


AOrderIcon::AOrderIcon()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	DisplayCountDown = 5.0f;
}


void AOrderIcon::BeginPlay()
{
	Super::BeginPlay();
}

void AOrderIcon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateToPlayer();
}

void AOrderIcon::RotateToPlayer()
{
	auto Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (!Player) {
		return;
	}

	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(WidgetComponent->GetComponentLocation(), Player->GetActorLocation());
	WidgetComponent->SetWorldRotation(TargetRotation);
}


void AOrderIcon::ShowIcon(bool CountdownHideIcon)
{
	// Clear countdown in case there is already a countdown that has begun
	GetWorldTimerManager().ClearTimer(THandler_Countdown);

	Root->SetVisibility(true, true);

	// set the visibility of the widget in order to trigger the visibility event used in the Widget blueprints
	if (WidgetComponent->GetUserWidgetObject()) {
		WidgetComponent->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Visible);
	}

	if (CountdownHideIcon) {
		GetWorldTimerManager().SetTimer(THandler_Countdown, this, &AOrderIcon::HideIcon, 1.0f, false, DisplayCountDown);
	}
}

void AOrderIcon::ShowIcon(EIconType SelectedIconType, bool CountdownHideIcon)
{
	switch (SelectedIconType)
	{
	case EIconType::Follow:
		WidgetComponent->SetWidgetClass(FollowWidgetClass);
		break;
	case EIconType::Attack:
		WidgetComponent->SetWidgetClass(AttackWidgetClass);
		break;
	case EIconType::Defend:
		WidgetComponent->SetWidgetClass(DefendWidgetClass);
		break;
	case EIconType::HVT:
		WidgetComponent->SetWidgetClass(HVTWidgetClass);
		break;
	case EIconType::Wounded:
		WidgetComponent->SetWidgetClass(WoundedWidgetClass);
		break;
	default:
		break;
	}

	ShowIcon(CountdownHideIcon);
}

void AOrderIcon::ShowIcon(FVector Location)
{
	SetActorLocation(Location);

	ShowIcon();
}

void AOrderIcon::HideIcon()
{
	// set the visibility of the widget in order to trigger the visibility event used in the Widget blueprints
	if (WidgetComponent->GetUserWidgetObject()) {
		WidgetComponent->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Collapsed);
	}

	Root->SetVisibility(false, true);
	GetWorldTimerManager().ClearTimer(THandler_Countdown);
}
