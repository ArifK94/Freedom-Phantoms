#include "GUI/OrderIcon.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"


AOrderIcon::AOrderIcon()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	Floor->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	Floor->SetCollisionProfileName(TEXT("NoCollision"));
	Floor->CanCharacterStepUpOn = ECB_No;
	Floor->SetGenerateOverlapEvents(false);

	Head = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Head"));
	Head->AttachToComponent(Floor, FAttachmentTransformRules::KeepRelativeTransform);
	Head->SetCollisionProfileName(TEXT("NoCollision"));
	Head->CanCharacterStepUpOn = ECB_No;
	Head->SetGenerateOverlapEvents(false);

	DisplayCountDown = 5.0f;
	TargetPosAmountZ = 3.0f;

}

void AOrderIcon::SetRotation(AActor* TargetActor)
{
	float x = 0.0f, y = 0.0f, Yaw = 0.0f;

	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(Head->GetComponentLocation(), TargetActor->GetActorLocation());
	UKismetMathLibrary::BreakRotator(TargetRotation, x, y, Yaw);
	TargetRotation = UKismetMathLibrary::MakeRotator(90.0f, 0.0f, Yaw + 90.0f);

	Head->SetWorldRotation(TargetRotation);

}

void AOrderIcon::BeginPlay()
{
	Super::BeginPlay();

	OrginalPos = Head->GetRelativeLocation();
	TargetPos = OrginalPos;
	TargetPos.Z = OrginalPos.Z * TargetPosAmountZ;
}

void AOrderIcon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurveTimeline.TickTimeline(DeltaTime);

	SetRotation(GetOwner());
}


void AOrderIcon::BeginCountDown()
{
	if (DisplayCountDown <= 0.0f) {
		return;
	}

	GetWorldTimerManager().SetTimer(THandler_Countdown, this, &AOrderIcon::HideIcon, 1.0f, false, DisplayCountDown);
}

// transitioning the head component back and fourth
void AOrderIcon::BeginAnimation(float Value)
{
	FVector handguardLoadTarget = UKismetMathLibrary::VLerp(OrginalPos, TargetPos, Value);
	Head->SetRelativeLocation(handguardLoadTarget);

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

void AOrderIcon::ShowIcon()
{
	// save CPU time if root is already visible
	if (Root->IsVisible()) {
		return;
	}

	Root->SetVisibility(true, true);

	// start the animation if the curve float has been provided
	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("BeginAnimation"));
		CurveTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
		CurveTimeline.SetLooping(false);
		CurveTimeline.SetPlayRate(1.0f / 2.0f);
		CurveTimeline.PlayFromStart();
	}

	BeginCountDown();
}

void AOrderIcon::ShowIcon(FVector Location)
{
	SetActorLocation(Location);

	ShowIcon();
}

void AOrderIcon::HideIcon()
{
	// save CPU time if root is already not visible
	if (!Root->IsVisible()) {
		return;
	}

	Root->SetVisibility(false, true);
	GetWorldTimerManager().ClearTimer(THandler_Countdown);
	CurveTimeline.Stop();
}
