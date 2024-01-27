// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomComponents/BattleChatterComponent.h"
#include "Controllers/CombatAIController.h"
#include "Characters/CombatCharacter.h"
#include "CustomComponents/TargetFinderComponent.h"
#include "CustomComponents/CoverFinderComponent.h"
#include "CustomComponents/HealthComponent.h"
#include "CustomComponents/TeamFactionComponent.h"
#include "Weapons/Weapon.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UBattleChatterComponent::UBattleChatterComponent()
{
}

void UBattleChatterComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBattleChatterComponent::TimerTick()
{
	Super::TimerTick();

	if (!CombatAIController)
	{
		CombatAIController = Cast<ACombatAIController>(GetOwner());

		if (CombatAIController)
		{
			CombatAIController->GetTargetFinderComponent()->OnTargetSearch.AddDynamic(this, &UBattleChatterComponent::OnTargetSearchUpdate);
			CombatAIController->GetCoverFinderComponent()->OnCoverSearch.AddDynamic(this, &UBattleChatterComponent::OnCoverSearchUpdate);
		}
	}
	else
	{
		if (!CombatCharacter)
		{
			APawn* Pawn = CombatAIController->GetPawn();

			if (!Pawn) {
				return;
			}

			CombatCharacter = Cast<ACombatCharacter>(Pawn);

			if (CombatCharacter)
			{
				VoiceClipsSet = CombatCharacter->GetVoiceClipsSet();

				AddBattleChatterItem();

				CombatCharacter->GetHealthComp()->OnHealthChanged.AddDynamic(this, &UBattleChatterComponent::OnHealthUpdate);
				CombatCharacter->OnMountedGunEnabled.AddDynamic(this, &UBattleChatterComponent::OnMountedGunEnabledUpdate);
				CombatCharacter->GetPrimaryWeapon()->OnWeaponUpdate.AddDynamic(this, &UBattleChatterComponent::OnWeaponUpdate);
				CombatCharacter->GetSecondaryWeaponObj()->OnWeaponUpdate.AddDynamic(this, &UBattleChatterComponent::OnWeaponUpdate);
			}
		}
	}
}

void UBattleChatterComponent::OnTargetSearchUpdate(FTargetSearchParameters TargetSearchParameters)
{
	if (TargetSearchParameters.TargetActor)
	{
		PlayTargetFound();
	}
}

void UBattleChatterComponent::OnCoverSearchUpdate(FCoverSearchParameters CoverSearchParameters)
{
	if (!CombatCharacter || !CombatAIController) {
		return;
	}

	if (CoverSearchParameters.IsCoverFound && !CombatCharacter->IsTakingCover() && CombatAIController->GetEnemyActor())
	{
		PlayCoverMe();
	}
}

void UBattleChatterComponent::OnMountedGunEnabledUpdate(AMountedGun* MountedGun)
{
	if (!CombatCharacter) {
		return;
	}

	if (CombatCharacter->IsUsingMountedWeapon())
	{
		PlayStayAlert();
	}
}

void UBattleChatterComponent::OnHealthUpdate(FHealthParameters InHealthParameters)
{
	if (!InHealthParameters.AffectedHealthComponent->IsAlive() && CanPlay(3))
	{
		ACombatCharacter* FriendlyCharacter = FindNearChatableFriendly();

		if (FriendlyCharacter)
		{
			FChatableParams ChatableParams;
			ChatableParams.Sound = FriendlyCharacter->GetVoiceClipsSet()->FriendlyDownSound;

			SendReponseCall(FriendlyCharacter, ChatableParams);
		}
	}
}

void UBattleChatterComponent::OnWeaponUpdate(FWeaponUpdateParameters WeaponUpdateParameters)
{
	if (WeaponUpdateParameters.WeaponState == EWeaponState::Firing)
	{
		PlaySuppressingFire();
	}
}

void UBattleChatterComponent::OnCallerReceived_Implementation(FChatableParams ChatableParams)
{
	if (ChatableParams.Sound && !THandler_DelaySound.IsValid())
	{
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UBattleChatterComponent::PlayDelaySound, ChatableParams.Sound);
		GetOwner()->GetWorldTimerManager().SetTimer(THandler_DelaySound, Delegate, 1.0f, false, ChatableParams.PlayDelayTime);
	}
}

void UBattleChatterComponent::PlayTargetFound()
{
	if (!CombatAIController->GetCommander() || !CombatAIController->GetEnemyActor()) {
		return;
	}

	if (CombatAIController->GetCommander()->GetCurrentRecruit()->Recruit == CombatCharacter && CombatAIController->GetCurrentCommand() == CommanderOrders::Attack)
	{
		if (CombatAIController->GetCommander()->GetCurrentRecruit()->HighValueTarget == CombatAIController->GetEnemyActor())
		{
			CombatCharacter->PlayVoiceSound(GetBattleChatterSound(VoiceClipsSet->TargetFoundSound));
		}
	}
}

void UBattleChatterComponent::PlaySuppressingFire()
{
	if (!CanPlay(50)) {
		return;
	}

	auto Sound = GetBattleChatterSound(VoiceClipsSet->OrderActionSuppressSound);

	if (Sound)
	{
		ACombatCharacter* FriendlyCharacter = FindNearChatableFriendly();

		// ensure character is not alone otherwise character will be speaking to itself.
		if (FriendlyCharacter)
		{
			if (CombatAIController->GetEnemyActor() && CombatCharacter->IsFiring())
			{
				CombatCharacter->PlayVoiceSound(Sound);
			}
		}
	}

}

void UBattleChatterComponent::PlayMoveCombat()
{
	if (!CanPlay()) {
		return;
	}

	CombatCharacter->PlayVoiceSound(GetBattleChatterSound(VoiceClipsSet->OrderMoveCombatSound));
}

void UBattleChatterComponent::PlayCoverMe()
{
	if (!CanPlay(10) || !VoiceClipsSet->OrderActionCoverMeSound) {
		return;
	}

	auto Sound = GetBattleChatterSound(VoiceClipsSet->OrderActionCoverMeSound);

	if (Sound)
	{
		ACombatCharacter* FriendlyCharacter = FindNearChatableFriendly();

		if (FriendlyCharacter)
		{
			if (FriendlyCharacter->GetVoiceClipsSet()->AcknowledgeSound)
			{
				CombatCharacter->PlayVoiceSound(GetBattleChatterSound(VoiceClipsSet->OrderActionCoverMeSound));

				FChatableParams ChatableParams;
				ChatableParams.Sound = FriendlyCharacter->GetVoiceClipsSet()->AcknowledgeSound;
				ChatableParams.PlayDelayTime = VoiceClipsSet->OrderActionCoverMeSound->GetDuration();
				SendReponseCall(FriendlyCharacter, ChatableParams);
			}
		}
	}
}

void UBattleChatterComponent::PlayStayAlert()
{
	if (!CanPlay() || !VoiceClipsSet->StayAlertSound) {
		return;
	}

	auto Sound = GetBattleChatterSound(VoiceClipsSet->StayAlertSound);

	if (Sound)
	{
		ACombatCharacter* FriendlyCharacter = FindNearChatableFriendly();

		if (FriendlyCharacter)
		{
			if (FriendlyCharacter->GetVoiceClipsSet()->AcknowledgeSound)
			{
				CombatCharacter->PlayVoiceSound(GetBattleChatterSound(VoiceClipsSet->StayAlertSound));

				FChatableParams ChatableParams;
				ChatableParams.Sound = FriendlyCharacter->GetVoiceClipsSet()->AcknowledgeSound;
				ChatableParams.PlayDelayTime = VoiceClipsSet->StayAlertSound->GetDuration();

				SendReponseCall(FriendlyCharacter, ChatableParams);
			}
		}
	}

}

ACombatCharacter* UBattleChatterComponent::FindNearChatableFriendly()
{
#pragma region  Uncomment code to add later for opitmisation & remove the working code at the bottom.
	/**
* Uncomment code to add later for opitmisation & remove the working code at the bottom.
*/
//float Radius = 1000.f;

//// create a collision sphere
//FCollisionShape MyColSphere = FCollisionShape::MakeSphere(Radius);

//// create tarray for hit results
//TArray<FHitResult> OutHits;

///** Prevent processing the same overlapped actors */
//TArray<AActor*> DetectionActors;

//FCollisionQueryParams QueryParams;
//QueryParams.AddIgnoredActor(CombatCharacter);

//// check if something got hit in the sweep
//bool isHit = GetWorld()->SweepMultiByChannel(OutHits, CombatCharacter->GetActorLocation(), CombatCharacter->GetActorLocation(), 
//	FQuat::Identity, ECC_Visibility, MyColSphere, QueryParams);

//if (isHit)
//{
//	// loop through TArray
//	for (auto& Hit : OutHits)
//	{
//		AActor* HitActor = Hit.GetActor();

//		if (!HitActor) {
//			continue;
//		}

//		if (HitActor == CombatCharacter) {
//			continue;
//		}

//		if (DetectionActors.Contains(HitActor)) {
//			continue;
//		}

//		if (!UHealthComponent::IsActorAlive(HitActor)) {
//			continue;
//		}

//		if (!UTeamFactionComponent::IsFriendly(CombatCharacter, HitActor)) {
//			continue;
//		}

//		// Does actor implement the avoidable interface?
//		if (!HitActor->GetClass()->ImplementsInterface(UChatable::StaticClass())) {

//			// Does the actor's controller implement the avoidable interface then?
//			if (HitActor->GetInstigatorController() && !HitActor->GetInstigatorController()->GetClass()->ImplementsInterface(UChatable::StaticClass())) {

//				if (!HitActor->GetComponentByClass(UBattleChatterComponent::StaticClass())) {
//					continue;
//				}
//			}
//		}

//		DetectionActors.Add(HitActor);
//	}

//	if (DetectionActors.IsEmpty()) {
//		return nullptr;
//	}

//	AActor* RandomActor = DetectionActors[rand() % DetectionActors.Num()];

//	if (RandomActor)
//	{
//		return Cast<ACombatCharacter>(RandomActor);
//	}
//}
#pragma endregion


	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatCharacter::StaticClass(), Characters);
	ACombatCharacter* ClosestAlly = nullptr;

	for (AActor* Actor : Characters)
	{
		if (Actor == CombatCharacter) {
			continue;
		}

		if (!UHealthComponent::IsActorAlive(Actor)) {
			continue;
		}

		if (!UTeamFactionComponent::IsFriendly(CombatCharacter, Actor)) {
			continue;
		}

		auto Character = Cast<ACombatCharacter>(Actor);


		if (ClosestAlly == nullptr)
		{
			ClosestAlly = Character;
		}
		else
		{
			if (Character->GetDistanceTo(CombatCharacter) < ClosestAlly->GetDistanceTo(CombatCharacter))
			{
				ClosestAlly = Character;
			}
		}
	}

	return ClosestAlly;
}

bool UBattleChatterComponent::CanPlay(int max)
{
	return FMath::RandRange(0, max) == 1;
}

void UBattleChatterComponent::PlayDelaySound(USoundBase* Sound)
{
	if (CombatCharacter)
	{
		CombatCharacter->PlayVoiceSound(Sound);
	}

	if (GetOwner())
	{
		GetOwner()->GetWorldTimerManager().ClearTimer(THandler_DelaySound);
	}
}


USoundBase* UBattleChatterComponent::GetBattleChatterSound(USoundBase* Sound)
{
	if (!Sound || PreviousBattleSoundInserted == Sound) {
		return nullptr;
	}

	for (FBattleChatterParams ChatterParam : BattleChatters)
	{
		// if sound is found & there is currently no cooldown.
		if (ChatterParam.Sound == Sound && !ChatterParam.THandler_Cooldown.IsValid())
		{
			FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &UBattleChatterComponent::CooldownChatter, ChatterParam);
			GetOwner()->GetWorldTimerManager().SetTimer(ChatterParam.THandler_Cooldown, Delegate, 1.0f, false, ChatterParam.CooldownAmount);
			return ChatterParam.Sound;
		}
	}

	return nullptr;
}

void UBattleChatterComponent::CooldownChatter(FBattleChatterParams ChatterParam)
{
	for (FBattleChatterParams Chatter : BattleChatters)
	{
		if (Chatter.Sound == ChatterParam.Sound)
		{
			GetOwner()->GetWorldTimerManager().ClearTimer(Chatter.THandler_Cooldown);
			PreviousBattleSoundInserted = nullptr;
			break;
		}
	}
}

void UBattleChatterComponent::SendReponseCall(ACombatCharacter* Character, FChatableParams ChatableParams)
{
	if (Character->GetController())
	{
		auto BattleChatterComponent = Character->GetController()->GetComponentByClass(UBattleChatterComponent::StaticClass());
		if (BattleChatterComponent && BattleChatterComponent->GetClass()->ImplementsInterface(UChatable::StaticClass()))
		{
			IChatable::Execute_OnCallerReceived(BattleChatterComponent, ChatableParams);
		}
	}
}

void UBattleChatterComponent::AddBattleChatterItem()
{
	if (!VoiceClipsSet) {
		return;
	}

	FBattleChatterParams ChatterParam;
	ChatterParam.Sound = VoiceClipsSet->TargetFoundSound;
	BattleChatters.Add(ChatterParam);

	ChatterParam = FBattleChatterParams();
	ChatterParam.Sound = VoiceClipsSet->OrderActionSuppressSound;
	ChatterParam.CooldownAmount = FMath::RandRange(5.f, 10.f);
	BattleChatters.Add(ChatterParam);

	ChatterParam = FBattleChatterParams();
	ChatterParam.Sound = VoiceClipsSet->OrderMoveCombatSound;
	ChatterParam.CooldownAmount = FMath::RandRange(5.f, 10.f);
	BattleChatters.Add(ChatterParam);

	ChatterParam = FBattleChatterParams();
	ChatterParam.Sound = VoiceClipsSet->OrderActionCoverMeSound;
	ChatterParam.CooldownAmount = FMath::RandRange(5.f, 10.f);
	BattleChatters.Add(ChatterParam);

	ChatterParam = FBattleChatterParams();
	ChatterParam.Sound = VoiceClipsSet->StayAlertSound;
	ChatterParam.CooldownAmount = FMath::RandRange(5.f, 10.f);
	BattleChatters.Add(ChatterParam);

	ChatterParam = FBattleChatterParams();
	ChatterParam.Sound = VoiceClipsSet->OrderActionSuppressSound;
	ChatterParam.CooldownAmount = FMath::RandRange(10.f, 20.f);
	BattleChatters.Add(ChatterParam);
}