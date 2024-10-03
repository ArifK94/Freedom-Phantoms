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

	CurrentRecruitIndex = 0;
	MaxRecruits = 12;

	CanSearchRecruits = true;

	RecruitMessage = "Recruit Operative";
	ReviveMessage = "Revive Operative";
}

bool ACommanderCharacter::GetCanRecruit()
{
	return CanSearchRecruits && (ActiveRecruits.Num() - WoundedCount) < MaxRecruits;
}

void ACommanderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckRecruit();
}

void ACommanderCharacter::OnRecruitHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!InHealthParameters.AffectedHealthComponent->IsAlive())
	{
		UCommanderRecruit* AffectedRecruit = nullptr;
		int RecruitIndex = 0;


		for (int i = 0; i < ActiveRecruits.Num(); i++)
		{
			auto Recruit = ActiveRecruits[i];

			if (Recruit->Recruit == InHealthParameters.DamagedActor)
			{
				AffectedRecruit = Recruit;
				RecruitIndex = i;
				break;
			}
		}

		if (AffectedRecruit) {

			UpdateActiveRecruits();

			// If the current recruit is not alive, then move onto the next recruit
			if (CurrentRecruit && CurrentRecruit->Recruit == InHealthParameters.DamagedActor) {
				IncrementCurrentRecruit();
			}

			OnRecruitHealthChange.Broadcast(AffectedRecruit, RecruitIndex);
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
	FVector Start = FollowCamera->GetComponentLocation();

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
	if (!UHealthComponent::IsActorAlive(CurrentTargetActor) && !UHealthComponent::IsActorWounded(CurrentTargetActor)) {
		return;
	}

	if (!UTeamFactionComponent::IsFriendly(this, CurrentTargetActor)) {
		return;
	}

	auto Character = Cast<ACombatCharacter>(CurrentTargetActor);

	if (!Character) {
		return;
	}

	// should not be able to recruit a player controlled character
	if (Character->IsPlayerControlled()) {
		return;
	}

	// should not recruit if in a vehicle
	if (Character->GetIsInVehicle()) {
		return;
	}

	// check if wounded
	if (UHealthComponent::IsActorWounded(CurrentTargetActor))
	{
		CurrentMessage = ReviveMessage;
		PotentialRecruit = Character;
		PotentialRecruit->ShowCharacterOutline(true);
	}
	// if has not already been 
	else if (!GetRecruitInfo(CurrentTargetActor))
	{
		CurrentMessage = RecruitMessage;
		PotentialRecruit = Character;
		PotentialRecruit->ShowCharacterOutline(true);
	}
}

void ACommanderCharacter::ChangeCommander(ACommanderCharacter* NewCommander)
{
	if (!NewCommander) {
		return;
	}

	for (auto Recruit : ActiveRecruits)
	{
		Recruit->Recruit->setCommandingOfficer(NewCommander);
	}

	NewCommander->SetActiveRecruits(ActiveRecruits);
	NewCommander->SetCurrentRecruit(CurrentRecruit);
	NewCommander->SetCurrentRecruitIndex(CurrentRecruitIndex);
	NewCommander->SetWoundedCount(WoundedCount);

	OnCommanderChange.Broadcast(NewCommander);
}

void ACommanderCharacter::InteractWithOperative()
{
	if (PotentialRecruit == nullptr) {
		return;
	}

	if (UHealthComponent::IsActorWounded(PotentialRecruit))
	{
		ReviveFriendly();
	}
	else
	{
		RecruitFollower();
	}
	UpdateActiveRecruits();
}


void ACommanderCharacter::RecruitFollower()
{
	if (PotentialRecruit == nullptr) {
		return;
	}

	// remove wounded recruit and replace with the new recruit
	if (ActiveRecruits.Num() >= MaxRecruits) {
		RemoveWounded();
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
	follower->Recruit->GetHealthComp()->OnHealthChanged.AddDynamic(this, &ACommanderCharacter::OnRecruitHealthUpdate);
	follower->Recruit->OnKillConfirm.AddDynamic(this, &ACommanderCharacter::OnOperativeKillConfirm);

	ActiveRecruits.Add(follower);

	// If only one recruit, then make this the current recruit to order around
	if (CurrentRecruit == nullptr) {
		CurrentRecruitIndex = 0;
		CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];
	}

	PlayCommunicationSound(GetVoiceClipsSet()->RecruitSound, follower);

	ResetTargetActor();

	UpdateActiveRecruits();

	OnOrderSent.Broadcast(follower, ActiveRecruits.Num() - 1);
}

void ACommanderCharacter::ReviveFriendly()
{
	if (PotentialRecruit == nullptr) {
		return;
	}

	PotentialRecruit->SetIsReviving(true);
	PlayVoiceSound(GetVoiceClipsSet()->RevivingSound);

	auto Recruit = GetRecruitInfo(PotentialRecruit);

	// If the wounded recruit is already recruited, then return to follow order as default
	if (Recruit)
	{
		FollowSingle(Recruit);
	}
	else
	{
		// If there is a wounded recruit & active recruits has reached max along with wounded recruits
		// then remove the last recruit as the last index would hold the wounded recruit after sorting the Active Recruits list
		// Remove all of the wounded recruit's event handlers

		if (GetCanRecruit()) {
			RecruitFollower();
		}
	}

	if (CurrentRecruit == nullptr) {
		CurrentRecruitIndex = 0;
		CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];
	}

	UpdateActiveRecruits();

	OnOrderSent.Broadcast(CurrentRecruit, ActiveRecruits.Num() - 1);
}

void ACommanderCharacter::RemoveWounded()
{
	WoundedCount--;

	// get the last recruit, this would be the wounded recruit
	auto Index = ActiveRecruits.Num() - 1;

	ActiveRecruits[Index]->Recruit->GetHealthComp()->OnHealthChanged.RemoveDynamic(this, &ACommanderCharacter::OnRecruitHealthUpdate);
	ActiveRecruits[Index]->Recruit->OnKillConfirm.RemoveDynamic(this, &ACommanderCharacter::OnOperativeKillConfirm);
	ActiveRecruits[Index]->Recruit->setCommandingOfficer(nullptr);

	ActiveRecruits.RemoveAt(Index);
	OnRemoveRecruit.Broadcast(this, Index);
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
	if (!CurrentRecruit) {
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
			float Delay = .1f;
			for (int i = 0; i < ActiveRecruits.Num(); i++)
			{
				AttackSingle(ActiveRecruits[i], EnemyCharacter, HitResult, Delay);
				Delay += .1f;
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

void ACommanderCharacter::AttackSingle(UCommanderRecruit* Recruit, ABaseCharacter* EnemyCharacter, FHitResult HitResult, float Delay)
{
	if (!Recruit) {
		return;
	}

	if (!UHealthComponent::IsActorAlive(Recruit->Recruit)) {
		return;
	}

	Recruit->CurrentCommand = CommanderOrders::Attack;
	Recruit->Recruit->GetOverheadIcon()->ShowIcon(EIconType::Attack);

	if (EnemyCharacter && UHealthComponent::IsActorAlive(EnemyCharacter) && !UTeamFactionComponent::IsFriendly(this, EnemyCharacter)) // if hit result is an enemy character
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

	OrderRecruit(Recruit, Delay);
}

void ACommanderCharacter::DefendArea(bool CommandAll)
{
	if (!CurrentRecruit) {
		return;
	}

	if (CommandAll)
	{
		float Delay = .1f;
		for (int i = 0; i < ActiveRecruits.Num(); i++)
		{
			DefendAreaSingle(ActiveRecruits[i], Delay);
			Delay += .1f;
		}
	}
	else
	{
		DefendAreaSingle(CurrentRecruit);
		IncrementCurrentRecruit();
	}


	PlayCommunicationSound(GetVoiceClipsSet()->DefendSound, CurrentRecruit);
}

void ACommanderCharacter::DefendAreaSingle(UCommanderRecruit* Recruit, float Delay)
{
	if (!UHealthComponent::IsActorAlive(Recruit->Recruit)) {
		return;
	}

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

	OrderRecruit(Recruit, Delay);
}


void ACommanderCharacter::FollowCommander(bool CommandAll)
{
	if (!CurrentRecruit) {
		return;
	}

	if (CommandAll)
	{
		float Delay = .1f;
		for (int i = 0; i < ActiveRecruits.Num(); i++)
		{
			FollowSingle(ActiveRecruits[i], Delay);
			Delay += .1f;
		}
	}
	else
	{
		FollowSingle(CurrentRecruit);
		IncrementCurrentRecruit();
	}

	PlayCommunicationSound(GetVoiceClipsSet()->FollowSound, CurrentRecruit);
}


void ACommanderCharacter::FollowSingle(UCommanderRecruit* Recruit, float Delay)
{
	if (!UHealthComponent::IsActorAlive(Recruit->Recruit)) {
		return;
	}

	Recruit->CurrentCommand = CommanderOrders::Follow;

	Recruit->TargetLocation = GetActorLocation();

	Recruit->Recruit->GetOverheadIcon()->ShowIcon(EIconType::Follow);

	GetWorldTimerManager().ClearTimer(Recruit->THandler_OrderDelay);

	OrderRecruit(Recruit, Delay);
}

void ACommanderCharacter::OrderRecruit(UCommanderRecruit* RecruitInfo, float Delay)
{
	GetWorldTimerManager().ClearTimer(RecruitInfo->THandler_OrderDelay);
	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ACommanderCharacter::BroadcastOrderDelay, RecruitInfo);
	GetWorldTimerManager().SetTimer(RecruitInfo->THandler_OrderDelay, RespawnDelegate, 1.0f, false, Delay);
}

void ACommanderCharacter::BroadcastOrderDelay(UCommanderRecruit* RecruitInfo)
{
	OnOrderSent.Broadcast(RecruitInfo, CurrentRecruitIndex);
	GetWorldTimerManager().ClearTimer(RecruitInfo->THandler_OrderDelay);
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

	bool navResult = NavigationArea->ProjectPointToNavigation(Position, NavLocation, FVector(100.f, 100.f, 1000.f));

	return NavLocation;
}

// Remove dead recruits
// Remove any icons if recruit or HVT character are dead
void ACommanderCharacter::UpdateActiveRecruits()
{
	if (ActiveRecruits.Num() <= 0) {
		return;
	}

	int CountWounded = 0;

	for (int i = ActiveRecruits.Num() - 1; i > 0; i--)
	{
		UCommanderRecruit* Recruit = ActiveRecruits[i];
		ACombatCharacter* RecruitCharacter = Recruit->Recruit;

		bool IsWounded = UHealthComponent::IsActorWounded(RecruitCharacter);

		if (UHealthComponent::IsActorAlive(RecruitCharacter) || IsWounded)
		{
			if (IsWounded)
			{
				CountWounded++;
			}

			ABaseCharacter* TargetCharacter = Recruit->HighValueTarget;

			// check if HVT is alive
			// if not, then remove the overhead icon
			if (TargetCharacter != nullptr)
			{
				if (!UHealthComponent::IsActorAlive(TargetCharacter))
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
		}
	}

	WoundedCount = CountWounded;

	SortRecruitList();
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

/// <summary>
/// Shift all elements to the left
/// removing any dead/ empty UI recruit elements by shifting them to the right
/// </summary>
void ACommanderCharacter::SortRecruitList()
{
	for (int write = 0; write < ActiveRecruits.Num(); write++) {
		for (int sort = 0; sort < ActiveRecruits.Num() - 1; sort++) {

			// If current recruit is wounded & the next recruit is still alive
			// swap their index positions
			if (!UHealthComponent::IsActorAlive(ActiveRecruits[sort]->Recruit) &&
				UHealthComponent::IsActorAlive(ActiveRecruits[sort + 1]->Recruit))
			{
				auto temp = ActiveRecruits[sort + 1];
				ActiveRecruits[sort + 1] = ActiveRecruits[sort];
				ActiveRecruits[sort] = temp;
			}
		}
	}
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
	auto TargetIndex = -1;

	// find next alive recruit 
	for (int i = CurrentRecruitIndex + 1; i < ActiveRecruits.Num(); i++) {

		if (UHealthComponent::IsActorAlive(ActiveRecruits[i]->Recruit)) {
			TargetIndex = i;
			break;
		}
	}

	// if still not found another recruit, start again from the beginning up to the current index
	if (TargetIndex == -1) {

		for (int i = 0; i <= CurrentRecruitIndex; i++) {

			if (UHealthComponent::IsActorAlive(ActiveRecruits[i]->Recruit)) {
				TargetIndex = i;
				break;
			}
		}

	}

	CurrentRecruitIndex = TargetIndex;

	if (TargetIndex == -1) {
		CurrentRecruit = nullptr;
	}
	else {
		CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];
	}
}

void ACommanderCharacter::PlayCommunicationSound(USoundBase* SoundBase, UCommanderRecruit* TargetRecruit)
{
	if (!GetWorld() || !TargetRecruit || !SoundBase) {
		return;
	}

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
	GetWorldTimerManager().ClearTimer(TargetRecruit->THandler_ResponseSound);
}

void ACommanderCharacter::ResetTargetActor()
{
	if (PotentialRecruit != nullptr)
	{
		PotentialRecruit->ShowCharacterOutline(false);
		PotentialRecruit = nullptr;
	}
}