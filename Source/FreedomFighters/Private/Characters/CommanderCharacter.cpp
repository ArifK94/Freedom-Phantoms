// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CommanderCharacter.h"
#include "..\..\Public\Characters\CommanderCharacter.h"

#include "Managers/FactionManager.h"

#include "CustomComponents/HealthComponent.h"

#include "Containers/Array.h"
#include "Engine.h"


ACommanderCharacter::ACommanderCharacter()
{

}


void ACommanderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Recruit", IE_Pressed, this, &ACommanderCharacter::Recruit);

}



void ACommanderCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACommanderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckRecruit();
}


void ACommanderCharacter::CheckRecruit()
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	//QueryParams.bReturnPhysicalMaterial = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	auto MyLocation = this->GetActorLocation();
	FHitResult OutHit;
	FVector Start = this->GetActorLocation();

	FVector ForwardVector = FollowCamera->GetForwardVector();
	FVector End = ((ForwardVector * 500.0f) + Start);

	if (GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, QueryParams))
	{
		if (OutHit.bBlockingHit)
		{
			auto CurrentTargetActor = OutHit.GetActor();
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentTargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
			bool isFriendly = UHealthComponent::IsFriendly(this, CurrentTargetActor);

			if (CurrentHealth && CurrentHealth->IsAlive() && isFriendly && !IfAlreadyRecruited(CurrentTargetActor))
			{
				ResetTargetActor();

				CurrentCombatCharacter = Cast<ACombatCharacter>(CurrentTargetActor);
				CurrentCombatCharacter->ShowCharacterOutline(true);
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
	if (CurrentCombatCharacter != nullptr)
	{
		FCommanderFollower follower = FCommanderFollower();
		follower.Follower = CurrentCombatCharacter;
		follower.CurrentCommand = CommanderOrders::Follow;
		ActorFollowers.Add(follower);


		if (FactionObj != nullptr)
		{
			VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().RecruitSound;
			VoiceAudioComponent->Play();
		}

		if (CurrentCombatCharacter->getFactionObj() != nullptr)
		{
			CurrentCombatCharacter->getVoiceAudioComponent()->Sound = CurrentCombatCharacter->getFactionObj()->getSelectedVoiceClipSet().AcknowledgeCommandSound;
			CurrentCombatCharacter->getVoiceAudioComponent()->Play();
		}


		ResetTargetActor();
	}
}

bool ACommanderCharacter::IfAlreadyRecruited(AActor* TargetActor)
{
	for (auto follower : ActorFollowers)
	{
		if (follower.Follower == TargetActor)
			return true;
	}


	return false;
}

void ACommanderCharacter::ResetTargetActor()
{
	if (CurrentCombatCharacter != nullptr)
	{
		CurrentCombatCharacter->ShowCharacterOutline(false);
		CurrentCombatCharacter = nullptr;
	}
}
