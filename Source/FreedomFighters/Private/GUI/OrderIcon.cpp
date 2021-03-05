

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

	CanAnimate = true;

	DisplayCountDown = 5.0f;
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
}

void AOrderIcon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetRotation(GetOwner());

	if (CanAnimate && Root->IsVisible())
	{
		//FVector handguardLoadTarget = UKismetMathLibrary::VInterpTo(OrginalPos, FVector(0, 0, OrginalPos.Z * 10.0f), DeltaTime, 5.0f);
		//Head->SetRelativeLocation(handguardLoadTarget);
	}

}


void AOrderIcon::BeginCountDown()
{
	if (DisplayCountDown <= 0.0f) {
		return;
	}

	GetWorldTimerManager().SetTimer(THandler_Countdown, this, &AOrderIcon::HideIcon, 1.0f, false, DisplayCountDown);
}

void AOrderIcon::ShowIcon()
{
	// save CPU time if root is already visible
	if (Root->IsVisible()) {
		return;
	}

	Root->SetVisibility(true, true);

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
}
