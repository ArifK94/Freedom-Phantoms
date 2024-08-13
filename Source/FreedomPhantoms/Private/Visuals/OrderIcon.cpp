#include "Visuals/OrderIcon.h"
#include "Characters/BaseCharacter.h"

//#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"


AOrderIcon::AOrderIcon()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	AnimationRoot = CreateDefaultSubobject<USceneComponent>(TEXT("AnimationRoot"));
	AnimationRoot->SetupAttachment(RootComponent);

	//WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	//WidgetComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);

	DisplayCountDown = 5.0f;
	TargetPosAmountZ = 50.0f;
}


void AOrderIcon::BeginPlay()
{
	Super::BeginPlay();

	OrginalPos = AnimationRoot->GetRelativeLocation();
	TargetPos = OrginalPos;
	TargetPos.Z = OrginalPos.Z + TargetPosAmountZ; // animating only in the z axis

	AttackIcon = SpawnIcon(AttackIconClass);
	DefendIcon = SpawnIcon(DefendIconClass);
	FollowIcon = SpawnIcon(FollowIconClass);
	WoundedIcon = SpawnIcon(WoundedIconClass);
}

void AOrderIcon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurveTimeline.TickTimeline(DeltaTime);

	FacePlayer();
}

void AOrderIcon::FacePlayer()
{
	auto Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	if (!Player) {
		return;
	}

	auto PlayerCharacter = Cast<ABaseCharacter>(Player);

	if (!PlayerCharacter) {
		return;
	}

	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(AnimationRoot->GetComponentLocation(), PlayerCharacter->FollowCamera->GetComponentLocation());
	AnimationRoot->SetWorldRotation(TargetRotation);
}

AActor* AOrderIcon::SpawnIcon(TSubclassOf<AActor> IconClass)
{
	UWorld* World = GetWorld();

	if (!World) {
		return nullptr;
	}

	if (!IconClass) {
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	auto Icon = World->SpawnActor<AActor>(IconClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Icon)
	{
		Icon->SetActorHiddenInGame(true);
		Icon->SetHidden(true);
		Icon->SetActorEnableCollision(false);
		Icon->SetActorTickEnabled(false);

		Icon->AttachToComponent(AnimationRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Icons.Add(Icon);
	}

	return Icon;
}

void AOrderIcon::DisplayIcon(AActor* SelectedIcon)
{
	for (auto Icon : Icons)
	{
		if (Icon == SelectedIcon)
		{
			Icon->SetActorHiddenInGame(false);
			Icon->SetHidden(false);
		}
		else
		{
			Icon->SetActorHiddenInGame(true);
			Icon->SetHidden(true);
		}
	}
}

void AOrderIcon::BeginAnimation(float Value)
{
	AnimationRoot->SetRelativeLocation(UKismetMathLibrary::VLerp(OrginalPos, TargetPos, Value));

	// if reached the end then reverse
	if (Value >= .9f)
	{
		CurveTimeline.Reverse();
	}

	// if reached near the start again then start
	if (Value <= .1f)
	{
		CurveTimeline.Play();
	}
}

void AOrderIcon::ShowIcon(bool CountdownHideIcon)
{
	// Clear countdown in case there is already a countdown that has begun
	GetWorldTimerManager().ClearTimer(THandler_Countdown);

	Root->SetVisibility(true, true);

	// set the visibility of the widget in order to trigger the visibility event used in the Widget blueprints
	//if (WidgetComponent->GetUserWidgetObject()) {
	//	WidgetComponent->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Visible);
	//}

	// start the animation if the curve float has been provided
	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("BeginAnimation"));
		CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
		CurveTimeline.SetLooping(true);
		CurveTimeline.SetPlayRate(1.f);
		CurveTimeline.PlayFromStart();
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
		DisplayIcon(FollowIcon);
		//WidgetComponent->SetWidgetClass(FollowWidgetClass);
		break;
	case EIconType::Attack:
		DisplayIcon(AttackIcon);
		//WidgetComponent->SetWidgetClass(AttackWidgetClass);
		break;
	case EIconType::Defend:
		DisplayIcon(DefendIcon);
		//WidgetComponent->SetWidgetClass(DefendWidgetClass);
		break;
	case EIconType::Wounded:
		DisplayIcon(WoundedIcon);
		//WidgetComponent->SetWidgetClass(WoundedWidgetClass);
		break;
	default:
		break;
	}

	ShowIcon(CountdownHideIcon);
}

void AOrderIcon::ShowIcon(FVector Location, bool CountdownHideIcon)
{
	SetActorLocation(Location);

	ShowIcon(CountdownHideIcon);
}

void AOrderIcon::HideIcon()
{
	// set the visibility of the widget in order to trigger the visibility event used in the Widget blueprints
	//if (WidgetComponent->GetUserWidgetObject()) {
	//	WidgetComponent->GetUserWidgetObject()->SetVisibility(ESlateVisibility::Collapsed);
	//}

	Root->SetVisibility(false, true);
	GetWorldTimerManager().ClearTimer(THandler_Countdown);
}
