#include "Characters/CommanderCharacter.h"
#include "GUI/OrderIcon.h"
#include "Managers/FactionManager.h"
#include "CustomComponents/HealthComponent.h"

#include "Containers/Array.h"
#include "Engine.h"
#include "Components/WidgetComponent.h"
#include "NavigationSystem.h"


ACommanderCharacter::ACommanderCharacter()
{
	CurrentRecruitIndex = 0;
	MaxRecruits = 9;
}

void ACommanderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Recruit", IE_Pressed, this, &ACommanderCharacter::Recruit);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACommanderCharacter::Attack);
	PlayerInputComponent->BindAction("Defend", IE_Pressed, this, &ACommanderCharacter::DefendArea);
	PlayerInputComponent->BindAction("Follow", IE_Pressed, this, &ACommanderCharacter::FollowCommander);
}



void ACommanderCharacter::AddUIWidget()
{
	UWorld* World = GetWorld();

	if (!World) return;

	if (CommanderHUDWidgetClass)
	{
		CommanderHUDWidget = CreateWidget<UUserWidget>(World, CommanderHUDWidgetClass);

		if (CommanderHUDWidget)
		{
			CommanderHUDWidget->AddToViewport();
		}
	}
}

void ACommanderCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACommanderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ActiveRecruits.Num() <= MaxRecruits) {
		CheckRecruit();
	}
	UpdateActiveRecruits();
}


FHitResult ACommanderCharacter::GetCurrentTraceHit(float Length)
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;


	FHitResult OutHit;
	FVector Start = GetActorLocation();

	FVector ForwardVector = FollowCamera->GetForwardVector();
	FVector End = ((ForwardVector * Length) + Start);

	auto LineTrace = GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, QueryParams);

	if (LineTrace)
	{
		if (OutHit.bBlockingHit)
		{
			return OutHit;
		}
	}

	return FHitResult();
}



void ACommanderCharacter::CheckRecruit()
{
	FHitResult HitResult = GetCurrentTraceHit();
	if (HitResult.bBlockingHit)
	{
		auto CurrentTargetActor = HitResult.GetActor();
		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentTargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
		bool isFriendly = UHealthComponent::IsFriendly(this, CurrentTargetActor);

		if (CurrentHealth && CurrentHealth->IsAlive() && isFriendly && !IfAlreadyRecruited(CurrentTargetActor))
		{
			ResetTargetActor();

			PotentialRecruit = Cast<ACombatCharacter>(CurrentTargetActor);

			if (PotentialRecruit != nullptr && !PotentialRecruit->IsInHelicopter())
			{
				PotentialRecruit->ShowCharacterOutline(true);
				PotentialRecruit->setCommandingOfficer(this);
			}
		}
	}
	else
	{
		ResetTargetActor();
	}
}

void ACommanderCharacter::Recruit()
{
	if (PotentialRecruit != nullptr)
	{
		UCommanderRecruit* follower = NewObject<UCommanderRecruit>(this);
		follower->Recruit = PotentialRecruit;
		follower->CurrentCommand = CommanderOrders::Follow;
		follower->TargetLocation = GetActorLocation();

		SpawnIcon(AttackOverheadClass, follower->AttackOverheadIcon);
		SpawnIcon(AttackPositionIconClass, follower->AttackPositionIcon);
		SpawnIcon(HighValueTargetOverheadClass, follower->HighValueTargetOverheadIcon);
		SpawnIcon(DefendIconPositionClass, follower->DefendPositionIcon);
		SpawnIcon(DefendOverheadClass, follower->DefendOverheadIcon);
		SpawnIcon(FollowOverheadClass, follower->FollowOverheadIcon);

		TArray<AOrderIcon*> OrderIconArray;
		OrderIconArray.Add(follower->AttackPositionIcon);
		OrderIconArray.Add(follower->HighValueTargetOverheadIcon);
		OrderIconArray.Add(follower->DefendPositionIcon);

		TArray<AOrderIcon*> OverheadIconArray;
		OverheadIconArray.Add(follower->AttackOverheadIcon);
		OverheadIconArray.Add(follower->DefendOverheadIcon);
		OverheadIconArray.Add(follower->FollowOverheadIcon);

		// set the overhead to attach to its head socket location
		// so the position can follow the character without updating the position every frame
		FVector HeadLocation = follower->Recruit->GetMesh()->GetSocketLocation(follower->Recruit->GetHeadSocket());

		for (AOrderIcon* OverheadIcon : OverheadIconArray)
		{
			OverheadIcon->AttachToComponent(follower->Recruit->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
			OverheadIcon->SetActorLocation(HeadLocation);
		}

		follower->OrderIconArray = OrderIconArray;
		follower->OverheadIconArray = OverheadIconArray;

		// display the follow overhead icon each time someone has been recruited
		DisplayOverheadIcon(follower->FollowOverheadIcon, OverheadIconArray);

		ActiveRecruits.Add(follower);

		if (ActiveRecruits.Num() <= 0)
		{
			CurrentRecruitIndex = 0;
			CurrentRecruit = follower;
		}

		CurrentRecruit = follower;
		PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().RecruitSound, follower);

		ResetTargetActor();
	}
}



bool ACommanderCharacter::IfAlreadyRecruited(AActor* TargetActor)
{
	for (auto follower : ActiveRecruits)
	{
		if (follower->Recruit == TargetActor)
			return true;
	}

	return false;
}

UCommanderRecruit* ACommanderCharacter::GetRecruitInfo(AActor* TargetActor)
{
	for (auto follower : ActiveRecruits)
	{
		if (follower->Recruit == TargetActor)
		{
			return follower;
		}
	}

	return nullptr;
}

void ACommanderCharacter::Attack()
{
	if (ActiveRecruits.Num() <= 0) {
		return;
	}

	FHitResult HitResult = GetCurrentTraceHit(50000.0f);

	if (HitResult.bBlockingHit)
	{
		AActor* TargetActor = HitResult.GetActor();

		ABaseCharacter* EnemyCharacter = Cast<ABaseCharacter>(TargetActor);
		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(TargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
		bool isFriendly = UHealthComponent::IsFriendly(this, TargetActor);

		CurrentRecruit->CurrentCommand = CommanderOrders::Attack;
		DisplayOverheadIcon(CurrentRecruit->AttackOverheadIcon, CurrentRecruit->OverheadIconArray);

		if (EnemyCharacter && CurrentHealth && CurrentHealth->IsAlive() && !isFriendly) // if hit result is an enemy character
		{
			CurrentRecruit->HighValueTarget = EnemyCharacter;
			CurrentRecruit->TargetLocation = GetPositionToNav(TargetActor->GetActorLocation()).Location;


			// attach the HVT overhead icon to the target head, rather than updating the location every frame of the enemy's head position
			FVector HeadLocation = EnemyCharacter->GetMesh()->GetSocketLocation(EnemyCharacter->GetHeadSocket());
			CurrentRecruit->HighValueTargetOverheadIcon->AttachToComponent(EnemyCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
			DisplayPositionIcon(CurrentRecruit->HighValueTargetOverheadIcon, CurrentRecruit->OrderIconArray, HeadLocation);
		}
		else // show the attack position 
		{
			CurrentRecruit->HighValueTarget = nullptr;
			CurrentRecruit->TargetLocation = GetPositionToNav(HitResult.ImpactPoint).Location;

			DisplayPositionIcon(CurrentRecruit->AttackPositionIcon, CurrentRecruit->OrderIconArray, CurrentRecruit->TargetLocation);
		}
	}

	PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().AttackSound, CurrentRecruit);

	IncrementCurrentRecruit();
}

void ACommanderCharacter::DefendArea()
{
	if (ActiveRecruits.Num() <= 0) {
		return;
	}

	if (isAiming)
	{
		FHitResult HitResult = GetCurrentTraceHit(50000.0f);

		if (HitResult.bBlockingHit)
		{
			CurrentRecruit->TargetLocation = GetPositionToNav(HitResult.ImpactPoint).Location;
		}
	}
	else
	{
		CurrentRecruit->TargetLocation = GetPositionToNav(GetActorLocation()).Location;
	}

	PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().DefendSound, CurrentRecruit);

	CurrentRecruit->CurrentCommand = CommanderOrders::Defend;

	DisplayPositionIcon(CurrentRecruit->DefendPositionIcon, CurrentRecruit->OrderIconArray, CurrentRecruit->TargetLocation);
	DisplayOverheadIcon(CurrentRecruit->DefendOverheadIcon, CurrentRecruit->OverheadIconArray);

	IncrementCurrentRecruit();
}

void ACommanderCharacter::FollowCommander()
{
	if (ActiveRecruits.Num() > 0)
	{
		CurrentRecruit->CurrentCommand = CommanderOrders::Follow;

		PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().FollowSound, CurrentRecruit);

		CurrentRecruit->TargetLocation = GetActorLocation();

		DisplayOverheadIcon(CurrentRecruit->FollowOverheadIcon, CurrentRecruit->OverheadIconArray);

		IncrementCurrentRecruit();
	}
}

/// <summary>
/// Ensures the target position is on the navmesh as well as the icons being placed on the nav
/// </summary>
/// <param name="Position"></param>
/// <returns></returns>
FNavLocation ACommanderCharacter::GetPositionToNav(FVector Position)
{
	FNavLocation NavLocation;

	UNavigationSystemV1* NavigationArea = FNavigationSystem::GetCurrent<UNavigationSystemV1>(this);

	bool navResult = NavigationArea->ProjectPointToNavigation(Position, NavLocation);

	return NavLocation;
}

// Remove dead recuits
// Remove any icons if recuit or HVT character are dead
void ACommanderCharacter::UpdateActiveRecruits()
{
	if (ActiveRecruits.Num() <= 0) {
		return;
	}

	for (int i = 0; i < ActiveRecruits.Num(); i++)
	{
		UCommanderRecruit* Recruit = ActiveRecruits[i];
		ACombatCharacter* RecruitCharacter = Recruit->Recruit;


		UHealthComponent* RecruitHealth = Cast<UHealthComponent>(RecruitCharacter->GetComponentByClass(UHealthComponent::StaticClass()));

		if (RecruitHealth->IsAlive())
		{
			ABaseCharacter* TargetCharacter = Recruit->HighValueTarget;

			// check if HVT is alive
			// if not, then remove the overhead icon
			if (TargetCharacter != nullptr)
			{
				UHealthComponent* TargetHealth = Cast<UHealthComponent>(TargetCharacter->GetComponentByClass(UHealthComponent::StaticClass()));

				if (!TargetHealth->IsAlive())
				{
					// remove the HVT overhead icon from the target character's head position
					Recruit->HighValueTargetOverheadIcon->HideIcon();

					// remove target character from memory
					Recruit->HighValueTarget = nullptr;
				}
			}
		}
		else
		{
			// remove all icons on display, if any
			HideAllIcons(Recruit->OrderIconArray);
			HideAllIcons(Recruit->OverheadIconArray);

			ActiveRecruits.RemoveAt(i);
		}
	}
}

void ACommanderCharacter::SpawnIcon(TSubclassOf<AOrderIcon> IconClass, AOrderIcon*& Icon)
{
	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Icon = World->SpawnActor<AOrderIcon>(IconClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Icon)
	{
		Icon->HideIcon();
	}
}


void ACommanderCharacter::DisplayPositionIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons, FVector Location)
{
	for (AOrderIcon* Icon : Icons)
	{
		if (Icon == SelectedIcon)
		{
			Icon->ShowIcon(Location);
		}
		else
		{
			Icon->HideIcon();
		}
	}
}

void ACommanderCharacter::DisplayOverheadIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons)
{
	for (AOrderIcon* Icon : Icons)
	{
		if (Icon == SelectedIcon)
		{
			Icon->ShowIcon();
		}
		else
		{
			Icon->HideIcon();
		}
	}
}

void ACommanderCharacter::HideAllIcons(TArray<AOrderIcon*> Icons)
{
	for (AOrderIcon* Icon : Icons)
	{
		Icon->HideIcon();
	}
}

void ACommanderCharacter::IncrementCurrentRecruit()
{
	if (CurrentRecruitIndex < ActiveRecruits.Num() - 1)
	{
		CurrentRecruitIndex++;
	}
	else
	{
		CurrentRecruitIndex = 0;
	}

	CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];

}

void ACommanderCharacter::PlayVoiceSound(USoundBase* SoundBase, UCommanderRecruit* TargetRecruit)
{
	// Play acknowledged sound after commander's voice sound has finished playing the order
	if (FactionObj != nullptr && SoundBase != nullptr)
	{
		VoiceAudioComponent->Sound = SoundBase;
		VoiceAudioComponent->Play();
	}

	GetWorldTimerManager().ClearTimer(TargetRecruit->THandler_ResponseSound);
	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ACommanderCharacter::PlayAcknowledgeSound, TargetRecruit);
	GetWorldTimerManager().SetTimer(TargetRecruit->THandler_ResponseSound, RespawnDelegate, 1.0f, false, SoundBase->GetDuration());
}

void ACommanderCharacter::PlayAcknowledgeSound(UCommanderRecruit* TargetRecruit)
{
	TargetRecruit->Recruit->getVoiceAudioComponent()->Sound = TargetRecruit->Recruit->getFactionObj()->getSelectedVoiceClipSet().AcknowledgeCommandSound;
	TargetRecruit->Recruit->getVoiceAudioComponent()->Play();
}

void ACommanderCharacter::ResetTargetActor()
{
	if (PotentialRecruit != nullptr)
	{
		PotentialRecruit->ShowCharacterOutline(false);
		PotentialRecruit = nullptr;
	}
}