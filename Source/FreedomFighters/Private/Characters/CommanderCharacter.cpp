// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CommanderCharacter.h"
#include "..\..\Public\Characters\CommanderCharacter.h"
#include "GUI/OrderIcon.h"

#include "Managers/FactionManager.h"

#include "CustomComponents/HealthComponent.h"

#include "Containers/Array.h"
#include "Engine.h"

#include "Kismet/KismetSystemLibrary.h"



ACommanderCharacter::ACommanderCharacter()
{
	CurrentRecruitIndex = 0;
}




void ACommanderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Recruit", IE_Pressed, this, &ACommanderCharacter::Recruit);
	PlayerInputComponent->BindAction("DefendArea", IE_Pressed, this, &ACommanderCharacter::DefendArea);
}



void ACommanderCharacter::BeginPlay()
{
	Super::BeginPlay();

	VoiceAudioComponent->OnAudioFinished.AddDynamic(this, &ACommanderCharacter::OnAudioFinished);
}

void ACommanderCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckRecruit();
}


FHitResult ACommanderCharacter::GetCurrentTraceHit(float Length)
{
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery2);
	ObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery_MAX);
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<AActor*> ActorsToIgnore;

	auto MyLocation = this->GetActorLocation();
	FHitResult OutHit;
	FVector Start = this->GetActorLocation();

	FVector ForwardVector = FollowCamera->GetForwardVector();
	FVector End = ((ForwardVector * Length) + Start);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);


	//auto SphereLineTrace = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), Start, End, 50.0f, ObjectTypes, true, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHit, true);

	auto LineTrace = GetWorld()->LineTraceSingleByObjectType(OutHit, Start, End, ObjectParams, QueryParams);

	if (LineTrace)
	{
		if (OutHit.bBlockingHit)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *OutHit.GetActor()->GetName()));
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Impact Point: %s"), *OutHit.ImpactPoint.ToString()));
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Normal Point: %s"), *OutHit.ImpactNormal.ToString()));

			return OutHit;
		}
	}


	return FHitResult();
}



void ACommanderCharacter::CheckRecruit()
{
	if (GetCurrentTraceHit().bBlockingHit)
	{

		auto CurrentTargetActor = GetCurrentTraceHit().GetActor();
		UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentTargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
		bool isFriendly = UHealthComponent::IsFriendly(this, CurrentTargetActor);

		if (CurrentHealth && CurrentHealth->IsAlive() && isFriendly && !IfAlreadyRecruited(CurrentTargetActor))
		{
			ResetTargetActor();

			PotentialRecruit = Cast<ACombatCharacter>(CurrentTargetActor);
			PotentialRecruit->ShowCharacterOutline(true);
			PotentialRecruit->setCommandingOfficer(this);
			LastRecruit = PotentialRecruit;
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
		FCommanderRecruit follower = FCommanderRecruit();
		follower.Recruit = PotentialRecruit;
		follower.CurrentCommand = CommanderOrders::Follow;
		ActiveRecruits.Add(follower);

		if (ActiveRecruits.Num() == 0)
		{
			CurrentRecruitIndex = 0;
			CurrentRecruit = follower;
		}

		if (FactionObj != nullptr)
		{
			VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().RecruitSound;
			VoiceAudioComponent->Play();
		}

		ResetTargetActor();
	}
}

void ACommanderCharacter::RecruitPlaySound()
{
}

bool ACommanderCharacter::IfAlreadyRecruited(AActor* TargetActor)
{
	for (auto follower : ActiveRecruits)
	{
		if (follower.Recruit == TargetActor)
			return true;
	}

	return false;
}

FCommanderRecruit ACommanderCharacter::GetRecruitInfo(AActor* TargetActor)
{
	for (auto follower : ActiveRecruits)
	{
		if (follower.Recruit == TargetActor)
		{
			return follower;
		}
	}

	return FCommanderRecruit();
}

void ACommanderCharacter::DefendArea()
{
	if (ActiveRecruits.Num() > 0)
	{
		FHitResult HitResult = GetCurrentTraceHit(50000.0f);

		if (HitResult.bBlockingHit)
		{
			if (FactionObj != nullptr)
			{
				VoiceAudioComponent->Sound = FactionObj->getSelectedVoiceClipSet().DefendSound;
				VoiceAudioComponent->Play();
			}

			ActiveRecruits[CurrentRecruitIndex].CurrentCommand = CommanderOrders::Defend;
			CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];

			TargetDefendLocation = HitResult.ImpactPoint;

			SpawnIcon();

			IncrementCurrentRecruit();

			//VoiceAudioComponent->Sound->GetDuration();
		}
	}

}

void ACommanderCharacter::SpawnIcon()
{
	if (HasOrderIcon())
	{
		OrderIconArray[CurrentRecruitIndex]->ShowIcon(TargetDefendLocation);
	}
	else
	{
		UWorld* world = GetWorld();

		if (world)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			OrderIconObj = world->SpawnActor<AOrderIcon>(OrderIcon, TargetDefendLocation, FRotator::ZeroRotator, SpawnParams);

			if (OrderIconObj)
			{
				OrderIconArray.Add(OrderIconObj);
				OrderIconObj->ShowIcon(TargetDefendLocation);
			}
		}
	}

}

bool ACommanderCharacter::HasOrderIcon()
{
	if (OrderIconArray.Num() == ActiveRecruits.Num())
	{
		for (int i = 0; i < OrderIconArray.Num(); i++)
		{
			if (OrderIconArray[CurrentRecruitIndex] == OrderIconArray[i])
			{
				return true;
			}
		}
	}

	return false;
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

}

void ACommanderCharacter::ResetTargetActor()
{
	if (PotentialRecruit != nullptr)
	{
		PotentialRecruit->ShowCharacterOutline(false);
		PotentialRecruit = nullptr;
	}
}


void ACommanderCharacter::OnAudioFinished()
{
	if (LastRecruit != nullptr)
	{
		if (LastRecruit->getFactionObj() != nullptr)
		{
			LastRecruit->getVoiceAudioComponent()->Sound = LastRecruit->getFactionObj()->getSelectedVoiceClipSet().AcknowledgeCommandSound;
			LastRecruit->getVoiceAudioComponent()->Play();
			LastRecruit = nullptr;
		}
	}
}