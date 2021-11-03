#include "Characters/CommanderCharacter.h"
#include "GUI/OrderIcon.h"
#include "CustomComponents/HealthComponent.h"

#include "Containers/Array.h"
#include "Engine.h"
#include "Components/WidgetComponent.h"
#include "NavigationSystem.h"


ACommanderCharacter::ACommanderCharacter()
{
	CurrentRecruitIndex = 0;
	MaxRecruits = 12;

	CanRecruit = true;
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

void ACommanderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CanRecruit && ActiveRecruits.Num() <= MaxRecruits) {
		CheckRecruit();
	}

	UpdateActiveRecruits();
}

void ACommanderCharacter::OnOperativeKillConfirm(int KillCount)
{
	OperativeKillCounter += KillCount;
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
	if (HitResult.GetActor())
	{
		ResetTargetActor();

		AActor* CurrentTargetActor = HitResult.GetActor();
		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentTargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
		bool isFriendly = UHealthComponent::IsFriendly(this, CurrentTargetActor);

		if (CurrentHealth && CurrentHealth->IsAlive() && isFriendly && !IfAlreadyRecruited(CurrentTargetActor))
		{
			auto Character = Cast<ACombatCharacter>(CurrentTargetActor);

			if (Character && !Character->IsPlayerControlled() && !Character->GetIsInAircraft()) // if not controlled by player
			{
				PotentialRecruit = Character;
				PotentialRecruit->ShowCharacterOutline(true);
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
	if (PotentialRecruit == nullptr) {
		return;
	}

	UCommanderRecruit* follower = NewObject<UCommanderRecruit>(this);
	follower->Recruit = PotentialRecruit;
	follower->Recruit->setCommandingOfficer(this);
	follower->CurrentCommand = CommanderOrders::Follow; // follow command by default
	follower->TargetLocation = GetActorLocation();
	follower->Recruit->OnKillConfirm.AddDynamic(this, &ACommanderCharacter::OnOperativeKillConfirm);

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

	// If only one recruit, then make this the current recruit to order around
	if (ActiveRecruits.Num() <= 1)
	{
		CurrentRecruitIndex = 0;
		CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];
	}

	PlayCommunicationSound(GetVoiceClipsSet()->RecruitSound, follower);

	ResetTargetActor();
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

void ACommanderCharacter::Attack(bool CommandAll)
{
	if (ActiveRecruits.Num() <= 0) {
		return;
	}

	FHitResult HitResult = GetCurrentTraceHit(50000.0f);

	if (HitResult.bBlockingHit)
	{
		AActor* TargetActor = HitResult.GetActor();

		ABaseCharacter* EnemyCharacter = Cast<ABaseCharacter>(TargetActor);
		UHealthComponent* TargetHealth = Cast<UHealthComponent>(TargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
		bool isFriendly = UHealthComponent::IsFriendly(this, TargetActor);

		if (CommandAll)
		{
			for (int i = 0; i < ActiveRecruits.Num(); i++)
			{
				AttackSingle(ActiveRecruits[i], EnemyCharacter, TargetHealth, isFriendly, HitResult);
			}
		}
		else
		{
			AttackSingle(CurrentRecruit, EnemyCharacter, TargetHealth, isFriendly, HitResult);
			IncrementCurrentRecruit();
		}
	}

	PlayCommunicationSound(GetVoiceClipsSet()->AttackSound, CurrentRecruit);

}

void ACommanderCharacter::AttackSingle(UCommanderRecruit* Recruit, ABaseCharacter* EnemyCharacter, UHealthComponent* TargetHealth, bool isFriendly, FHitResult HitResult)
{
	Recruit->CurrentCommand = CommanderOrders::Attack;
	DisplayOverheadIcon(Recruit->AttackOverheadIcon, Recruit->OverheadIconArray);

	if (EnemyCharacter && TargetHealth && TargetHealth->IsAlive() && !isFriendly) // if hit result is an enemy character
	{
		Recruit->HighValueTarget = EnemyCharacter;
		Recruit->TargetLocation = GetPositionToNav(EnemyCharacter->GetActorLocation()).Location;


		// attach the HVT overhead icon to the target head, rather than updating the location every frame of the enemy's head position
		FVector HeadLocation = EnemyCharacter->GetMesh()->GetSocketLocation(EnemyCharacter->GetHeadSocket());
		Recruit->HighValueTargetOverheadIcon->AttachToComponent(EnemyCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform);
		DisplayPositionIcon(Recruit->HighValueTargetOverheadIcon, Recruit->OrderIconArray, HeadLocation);
	}
	else // show the attack position 
	{
		Recruit->HighValueTarget = nullptr;
		Recruit->TargetLocation = GetPositionToNav(HitResult.ImpactPoint).Location;

		DisplayPositionIcon(Recruit->AttackPositionIcon, Recruit->OrderIconArray, Recruit->TargetLocation);
	}

	OnOrderSent.Broadcast(Recruit);

}

void ACommanderCharacter::DefendArea(bool CommandAll)
{
	if (ActiveRecruits.Num() <= 0) {
		return;
	}

	if (CommandAll)
	{
		for (int i = 0; i < ActiveRecruits.Num(); i++)
		{
			DefendAreaSingle(ActiveRecruits[i]);
		}
	}
	else
	{
		DefendAreaSingle(CurrentRecruit);
		IncrementCurrentRecruit();
	}


	PlayCommunicationSound(GetVoiceClipsSet()->DefendSound, CurrentRecruit);
}

void ACommanderCharacter::DefendAreaSingle(UCommanderRecruit* Recruit)
{
	if (isAiming)
	{
		FHitResult HitResult = GetCurrentTraceHit(50000.0f);

		if (HitResult.bBlockingHit)
		{
			Recruit->TargetLocation = GetPositionToNav(HitResult.ImpactPoint).Location;
		}
	}
	else
	{
		Recruit->TargetLocation = GetPositionToNav(GetActorLocation()).Location;
	}
	Recruit->CurrentCommand = CommanderOrders::Defend;
	DisplayPositionIcon(Recruit->DefendPositionIcon, Recruit->OrderIconArray, Recruit->TargetLocation);
	DisplayOverheadIcon(Recruit->DefendOverheadIcon, Recruit->OverheadIconArray);

	OnOrderSent.Broadcast(Recruit);
}


void ACommanderCharacter::FollowCommander(bool CommandAll)
{
	if (ActiveRecruits.Num() <= 0) {
		return;
	}

	if (CommandAll)
	{
		for (int i = 0; i < ActiveRecruits.Num(); i++)
		{
			FollowSingle(ActiveRecruits[i]);
		}
	}
	else
	{
		FollowSingle(CurrentRecruit);
		IncrementCurrentRecruit();
	}

	PlayCommunicationSound(GetVoiceClipsSet()->FollowSound, CurrentRecruit);
}


void ACommanderCharacter::FollowSingle(UCommanderRecruit* Recruit)
{
	Recruit->CurrentCommand = CommanderOrders::Follow;

	Recruit->TargetLocation = GetActorLocation();

	DisplayOverheadIcon(Recruit->FollowOverheadIcon, Recruit->OverheadIconArray);

	OnOrderSent.Broadcast(Recruit);
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

			// if the current recruit died,
			if (CurrentRecruit == Recruit)
			{
				// then update the current recruit to the next recruit in the list
				if (i + 1 < ActiveRecruits.Num())
				{
					CurrentRecruit = ActiveRecruits[i + 1];
				}
				else // otherwise update the current recruit back to the first recruit in the list
				{
					// check if there is another recruit in the list 
					if (ActiveRecruits.Num() > 0)
					{
						CurrentRecruit = ActiveRecruits[0];
					}
					else
					{
						CurrentRecruit = nullptr;
					}
				}
			}

			SortActiveRecruits(i);
		}
	}
}

/// <summary>
/// Shift all elements to the left
/// removing any dead/ empty UI recruit elements by shifting them to the right
/// </summary>
/// <param name="StartingPoint"></param>
void ACommanderCharacter::SortActiveRecruits(int StartingPoint)
{
	// NewPosition is used to send the new index for the OnRemoveRecruit event to be used in the UI Widget since Current index is used to mark the current recruit on the UI
	int NewPosition = StartingPoint;

	for (int i = StartingPoint; i < ActiveRecruits.Num(); i++) // start the from index of the recruit who is dead
	{

		// When swapping, if the CurrentRecruitIndex is after the starting point then this needs to be updated
		if (CurrentRecruitIndex == i)
		{
			// update the current index & current recruit
			if (i - 1 > -1)
			{
				CurrentRecruitIndex = i - 1;
			}
			else
			{
				CurrentRecruitIndex = 0;
			}

			CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];
		}

		// Only swap if there is another element afterwards
		int nextElement = i + 1;
		if (nextElement < ActiveRecruits.Num())
		{
			ActiveRecruits.Swap(i, nextElement);
			NewPosition = nextElement;
		}

	}

	ActiveRecruits.RemoveAt(NewPosition);


	OnRemoveRecruit.Broadcast(this, NewPosition);
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

void ACommanderCharacter::PlayCommunicationSound(USoundBase* SoundBase, UCommanderRecruit* TargetRecruit)
{
	// Play acknowledged sound after commander's voice sound has finished playing the order
	PlayVoiceSound(SoundBase);


	GetWorldTimerManager().ClearTimer(TargetRecruit->THandler_ResponseSound);
	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ACommanderCharacter::PlayAcknowledgeSound, TargetRecruit);
	GetWorldTimerManager().SetTimer(TargetRecruit->THandler_ResponseSound, RespawnDelegate, 1.0f, false, SoundBase->GetDuration());
}

void ACommanderCharacter::PlayAcknowledgeSound(UCommanderRecruit* TargetRecruit)
{
	if (TargetRecruit == nullptr || TargetRecruit->Recruit == nullptr) {
		return;
	}

	TargetRecruit->Recruit->PlayVoiceSound(TargetRecruit->Recruit->GetVoiceClipsSet()->AcknowledgeCommandSound);
}

void ACommanderCharacter::ResetTargetActor()
{
	if (PotentialRecruit != nullptr)
	{
		PotentialRecruit->ShowCharacterOutline(false);
		PotentialRecruit = nullptr;
	}
}