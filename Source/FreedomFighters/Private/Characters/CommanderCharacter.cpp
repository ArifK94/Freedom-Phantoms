#include "Characters/CommanderCharacter.h"
#include "Visuals/OrderIcon.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "CustomComponents/TargetFinderComponent.h"

#include "Containers/Array.h"
#include "Engine.h"
#include "Components/WidgetComponent.h"
#include "NavigationSystem.h"


ACommanderCharacter::ACommanderCharacter()
{
	TargetSeekerComponent = CreateDefaultSubobject<UTargetFinderComponent>(TEXT("TargetSeekerComponent"));
	TargetSeekerComponent->SetCreateTargetSphere(false);

	CurrentRecruitIndex = 0;
	MaxRecruits = 12;

	CanRecruit = true;

	RecruitMessage = "Recruit Operative";
	ReviveMessage = "Revive Operative";
}

bool ACommanderCharacter::GetCanRecruit()
{
	return CanRecruit && (ActiveRecruits.Num() - WoundedCount) < MaxRecruits;
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

	CheckRecruit();
	UpdateActiveRecruits();
}

void ACommanderCharacter::OnRecruitHealthUpdate(FHealthParameters InHealthParameters)
{
	if (ActiveRecruits.Num() > 0 && !InHealthParameters.AffectedHealthComponent->GetIsWounded())
	{
		for (int i = 0; i < ActiveRecruits.Num(); i++)
		{
			auto Recruit = ActiveRecruits[i];
			if (Recruit->Recruit == InHealthParameters.DamagedActor)
			{
				if (InHealthParameters.AffectedHealthComponent->GetIsWounded())
				{
					SortActiveRecruits(i, false);
					WoundedCount++;
				}
				break;
			}
		}
	}
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

	ResetTargetActor();

	AActor* CurrentTargetActor = HitResult.GetActor();

	if (!CurrentTargetActor) {
		return;
	}


	if (CurrentTargetActor == this) {
		return;
	}

	// if not alive
	if (!UHealthComponent::IsAlive(CurrentTargetActor) && !UHealthComponent::IsWounded(CurrentTargetActor)) {
		return;
	}

	if (!UTeamFactionComponent::IsFriendly(this, CurrentTargetActor)) {
		return;
	}

	auto Character = Cast<ACombatCharacter>(CurrentTargetActor);

	if (!Character) {
		return;
	}

	if (Character->IsPlayerControlled()) {
		return;
	}

	// if controlled by player
	if (Character->GetIsInVehicle()) {
		return;
	}

	// check if wounded
	if (UHealthComponent::IsWounded(CurrentTargetActor)) 
	{
		CurrentMessage = ReviveMessage;
		PotentialRecruit = Character;
		PotentialRecruit->ShowCharacterOutline(true);
	}
	else
	{
		// if has not already been recruited
		if (!IfAlreadyRecruited(CurrentTargetActor))
		{
			CurrentMessage = RecruitMessage;
			PotentialRecruit = Character;
			PotentialRecruit->ShowCharacterOutline(true);
		}
	}
}

void ACommanderCharacter::InteractWithOperative()
{
	if (PotentialRecruit == nullptr) {
		return;
	}

	if (UHealthComponent::IsWounded(PotentialRecruit))
	{
		ReviveFriendly();
	}
	else
	{
		Recruit();
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


	SpawnIcon(AttackPositionIconClass, follower->AttackPositionIcon);
	SpawnIcon(DefendIconPositionClass, follower->DefendPositionIcon);
	SpawnIcon(HighValueTargetOverheadClass, follower->HighValueTargetOverheadIcon);

	TArray<AOrderIcon*> OrderIconArray;
	OrderIconArray.Add(follower->AttackPositionIcon);
	OrderIconArray.Add(follower->DefendPositionIcon);
	OrderIconArray.Add(follower->HighValueTargetOverheadIcon);

	follower->OrderIconArray = OrderIconArray;

	// display the follow overhead icon each time someone has been recruited
	follower->Recruit->GetOverheadIcon()->ShowIcon(EIconType::Follow);

	if (!follower->Recruit->GetHealthComp()->OnHealthChanged.IsBound()) {
		follower->Recruit->GetHealthComp()->OnHealthChanged.AddDynamic(this, &ACommanderCharacter::OnRecruitHealthUpdate);
	}

	if (!follower->Recruit->OnKillConfirm.IsBound()) {
		follower->Recruit->OnKillConfirm.AddDynamic(this, &ACommanderCharacter::OnOperativeKillConfirm);
	}

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

void ACommanderCharacter::ReviveFriendly()
{
	if (PotentialRecruit == nullptr) {
		return;
	}

	PotentialRecruit->SetIsReviving(true);

	if (IfAlreadyRecruited(PotentialRecruit))
	{
		PotentialRecruit->GetOverheadIcon()->ShowIcon(EIconType::Follow);
	}
	else
	{
		if (ActiveRecruits.Num() - WoundedCount >= MaxRecruits) {

			auto Index = ActiveRecruits.Num() - 1;

			if (ActiveRecruits[Index]->Recruit->GetHealthComp()->OnHealthChanged.IsBound()) {
				ActiveRecruits[Index]->Recruit->GetHealthComp()->OnHealthChanged.RemoveDynamic(this, &ACommanderCharacter::OnRecruitHealthUpdate);
			}

			if (!ActiveRecruits[Index]->Recruit->OnKillConfirm.IsBound()) {
				ActiveRecruits[Index]->Recruit->OnKillConfirm.RemoveDynamic(this, &ACommanderCharacter::OnOperativeKillConfirm);
			}

			ActiveRecruits.RemoveAt(Index);
			OnRemoveRecruit.Broadcast(this, Index);
		}

		Recruit();
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

void ACommanderCharacter::Attack(bool CommandAll)
{
	if (ActiveRecruits.Num() <= 0) {
		return;
	}

	FHitResult HitResult = GetCurrentTraceHit(9999999999999.0f);

	if (HitResult.bBlockingHit)
	{
		AActor* TargetActor = HitResult.GetActor();

		ABaseCharacter* EnemyCharacter = Cast<ABaseCharacter>(TargetActor);
		UHealthComponent* TargetHealth = Cast<UHealthComponent>(TargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
		bool IsTargetFactionCompActive = UTeamFactionComponent::IsComponentActive(TargetActor);
		bool isFriendly = UTeamFactionComponent::IsFriendly(this, TargetActor);

		if (CommandAll)
		{
			for (int i = 0; i < ActiveRecruits.Num(); i++)
			{
				AttackSingle(ActiveRecruits[i], EnemyCharacter, HitResult);
			}
		}
		else
		{
			AttackSingle(CurrentRecruit, EnemyCharacter, HitResult);
			IncrementCurrentRecruit();
		}
	}

	PlayCommunicationSound(GetVoiceClipsSet()->AttackSound, CurrentRecruit);
}

void ACommanderCharacter::AttackSingle(UCommanderRecruit* Recruit, ABaseCharacter* EnemyCharacter, FHitResult HitResult)
{
	if (!Recruit) {
		return;
	}

	Recruit->CurrentCommand = CommanderOrders::Attack;
	Recruit->Recruit->GetOverheadIcon()->ShowIcon(EIconType::Attack);

	if (EnemyCharacter && UHealthComponent::IsAlive(EnemyCharacter) && !UTeamFactionComponent::IsFriendly(this, EnemyCharacter)) // if hit result is an enemy character
	{
		Recruit->HighValueTarget = EnemyCharacter;
		Recruit->TargetLocation = EnemyCharacter->GetActorLocation();

		EnemyCharacter->AttachIconToHead(Recruit->HighValueTargetOverheadIcon);
		DisplayPositionIcon(Recruit->HighValueTargetOverheadIcon, Recruit->OrderIconArray, false);

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
	Recruit->Recruit->GetOverheadIcon()->ShowIcon(EIconType::Defend);

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

	Recruit->Recruit->GetOverheadIcon()->ShowIcon(EIconType::Follow);

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

		if (UHealthComponent::IsAlive(RecruitCharacter) || UHealthComponent::IsWounded(RecruitCharacter))
		{
			ABaseCharacter* TargetCharacter = Recruit->HighValueTarget;

			// check if HVT is alive
			// if not, then remove the overhead icon
			if (TargetCharacter != nullptr)
			{
				if (!UHealthComponent::IsAlive(TargetCharacter))
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

			SortActiveRecruits(i, true);
		}
	}
}

/// <summary>
/// Shift all elements to the left
/// removing any dead/ empty UI recruit elements by shifting them to the right
/// </summary>
/// <param name="StartingPoint"></param>
void ACommanderCharacter::SortActiveRecruits(int StartingPoint, bool RemoveIndex)
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

	if (RemoveIndex) {
		ActiveRecruits.RemoveAt(NewPosition);
	}
	OnRemoveRecruit.Broadcast(this, NewPosition);
}

void ACommanderCharacter::SpawnIcon(TSubclassOf<AOrderIcon> IconClass, AOrderIcon*& Icon)
{
	UWorld* World = GetWorld();

	if (!World) {
		return;
	}

	if (!IconClass) {
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


void ACommanderCharacter::DisplayPositionIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons, FVector Location, bool CountdownHideIcon)
{
	for (AOrderIcon* Icon : Icons)
	{
		if (Icon == SelectedIcon)
		{
			Icon->ShowIcon(Location, CountdownHideIcon);
		}
		else
		{
			Icon->HideIcon();
		}
	}
}

void ACommanderCharacter::DisplayPositionIcon(AOrderIcon* SelectedIcon, TArray<AOrderIcon*> Icons, bool CountdownHideIcon)
{
	for (AOrderIcon* Icon : Icons)
	{
		if (Icon == SelectedIcon)
		{
			Icon->ShowIcon(CountdownHideIcon);
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