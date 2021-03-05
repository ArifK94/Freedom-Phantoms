// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CommanderCharacter.h"
#include "GUI/OrderIcon.h"

#include "Managers/FactionManager.h"

#include "CustomComponents/HealthComponent.h"

#include "Containers/Array.h"
#include "Engine.h"
#include "Components/WidgetComponent.h"

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

	UpdateOverheadIcon();
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
		AActor* CurrentTargetActor = HitResult.GetActor();

		ABaseCharacter* Character = Cast<ABaseCharacter>(CurrentTargetActor);
		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentTargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
		bool isFriendly = UHealthComponent::IsFriendly(this, CurrentTargetActor);

		CurrentRecruit->CurrentCommand = CommanderOrders::Attack;
		DisplayOverheadIcon(CurrentRecruit->AttackOverheadIcon, CurrentRecruit->OverheadIconArray);

		if (Character && CurrentHealth && CurrentHealth->IsAlive() && !isFriendly) // if hit result is an enemy character
		{
			CurrentRecruit->HighValueTarget = Character;
			CurrentRecruit->TargetLocation = CurrentTargetActor->GetActorLocation();

			DisplayPositionIcon(CurrentRecruit->HighValueTargetOverheadIcon, CurrentRecruit->OrderIconArray, CurrentRecruit->TargetLocation);
		}
		else // show the attack position 
		{
			CurrentRecruit->HighValueTarget = nullptr;
			CurrentRecruit->TargetLocation = HitResult.ImpactPoint;

			DisplayPositionIcon(CurrentRecruit->AttackPositionIcon, CurrentRecruit->OrderIconArray, CurrentRecruit->TargetLocation);
		}
	}

	PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().AttackSound, CurrentRecruit);

	IncrementCurrentRecruit();
}

// overhead icons need to be updated to get the respective character's position every frame
void ACommanderCharacter::UpdateOverheadIcon()
{
	if (ActiveRecruits.Num() > 0)
	{
		for (int i = 0; i < ActiveRecruits.Num(); i++)
		{

			if (ActiveRecruits[i]->CurrentCommand == CommanderOrders::Attack)
			{
				ABaseCharacter* TargetCharacter = ActiveRecruits[i]->HighValueTarget;

				if (TargetCharacter != nullptr)
				{
					UHealthComponent* TargetHealth = Cast<UHealthComponent>(TargetCharacter->GetComponentByClass(UHealthComponent::StaticClass()));

					if (TargetHealth->IsAlive())
					{
						FVector HeadLocation = TargetCharacter->GetMesh()->GetSocketLocation(TargetCharacter->GetHeadSocket());

						//CurrentRecruit->HighValueTargetOverheadIcon->ShowIcon(HeadLocation);
					}
					else
					{
						//ActiveRecruits[i]->HighValueTargetOverheadIcon->HideIcon();
					}
				}
			}
		}
	}
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
			CurrentRecruit->TargetLocation = HitResult.ImpactPoint;
		}
	}
	else
	{
		CurrentRecruit->TargetLocation = GetActorLocation();
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