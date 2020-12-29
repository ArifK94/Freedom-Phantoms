// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CommanderCharacter.h"
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
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ACommanderCharacter::Attack);
	PlayerInputComponent->BindAction("Defend", IE_Pressed, this, &ACommanderCharacter::DefendArea);
	PlayerInputComponent->BindAction("Follow", IE_Pressed, this, &ACommanderCharacter::FollowCommander);
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

	FVector MyLocation = this->GetActorLocation();
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
				LastRecruit = PotentialRecruit;

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
		FCommanderRecruit follower = FCommanderRecruit();
		follower.Recruit = PotentialRecruit;
		follower.CurrentCommand = CommanderOrders::Follow;
		follower.TargetLocation = GetActorLocation();

		ActiveRecruits.Add(follower);

		if (ActiveRecruits.Num() == 0)
		{
			CurrentRecruitIndex = 0;
			CurrentRecruit = follower;
		}

		PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().RecruitSound, ActiveRecruits[CurrentRecruitIndex]);

		ResetTargetActor();
	}
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

void ACommanderCharacter::Attack()
{
	if (ActiveRecruits.Num() > 0)
	{
		FHitResult HitResult = GetCurrentTraceHit(50000.0f);

		if (HitResult.bBlockingHit)
		{
			AActor* CurrentTargetActor = HitResult.GetActor();
			UHealthComponent* CurrentHealth = Cast<UHealthComponent>(CurrentTargetActor->GetComponentByClass(UHealthComponent::StaticClass()));
			bool isFriendly = UHealthComponent::IsFriendly(this, CurrentTargetActor);

			if (CurrentHealth && CurrentHealth->IsAlive() && !isFriendly)
			{
				ActiveRecruits[CurrentRecruitIndex].TargetLocation = CurrentTargetActor->GetActorLocation();
			}
			else
			{
				ActiveRecruits[CurrentRecruitIndex].TargetLocation = HitResult.ImpactPoint;
			}
		}

		ActiveRecruits[CurrentRecruitIndex].CurrentCommand = CommanderOrders::Attack;
		CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];


		SpawnIcon(AttackMaterial);

		PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().AttackSound, ActiveRecruits[CurrentRecruitIndex]);

		IncrementCurrentRecruit();
	}
}

void ACommanderCharacter::DefendArea()
{
	if (ActiveRecruits.Num() > 0)
	{
		if (isAiming)
		{
			FHitResult HitResult = GetCurrentTraceHit(50000.0f);

			if (HitResult.bBlockingHit)
			{
				ActiveRecruits[CurrentRecruitIndex].TargetLocation = HitResult.ImpactPoint;
			}
		}
		else
		{
			ActiveRecruits[CurrentRecruitIndex].TargetLocation = GetActorLocation();
		}

		PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().DefendSound, ActiveRecruits[CurrentRecruitIndex]);

		ActiveRecruits[CurrentRecruitIndex].CurrentCommand = CommanderOrders::Defend;
		CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];


		SpawnIcon(DefendMaterial);

		IncrementCurrentRecruit();
	}

}

void ACommanderCharacter::FollowCommander()
{
	if (ActiveRecruits.Num() > 0)
	{
		ActiveRecruits[CurrentRecruitIndex].CurrentCommand = CommanderOrders::Follow;
		CurrentRecruit = ActiveRecruits[CurrentRecruitIndex];

		PlayVoiceSound(FactionObj->getSelectedVoiceClipSet().FollowSound, ActiveRecruits[CurrentRecruitIndex]);

		ActiveRecruits[CurrentRecruitIndex].TargetLocation = GetActorLocation();

		IncrementCurrentRecruit();
	}
}

void ACommanderCharacter::SpawnIcon(UMaterialInterface* Material)
{
	if (HasOrderIcon())
	{
		OrderIconArray[CurrentRecruitIndex]->SetIconMaterial(Material);
		OrderIconArray[CurrentRecruitIndex]->ShowIcon(ActiveRecruits[CurrentRecruitIndex].TargetLocation);
	}
	else
	{
		UWorld* world = GetWorld();

		if (world)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			OrderIconObj = world->SpawnActor<AOrderIcon>(OrderIcon, ActiveRecruits[CurrentRecruitIndex].TargetLocation, FRotator::ZeroRotator, SpawnParams);

			if (OrderIconObj)
			{
				OrderIconArray.Add(OrderIconObj);
				OrderIconObj->SetIconMaterial(Material);
				OrderIconObj->ShowIcon(ActiveRecruits[CurrentRecruitIndex].TargetLocation);
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

void ACommanderCharacter::PlayVoiceSound(USoundBase* SoundBase, FCommanderRecruit TargetRecruit)
{
	if (FactionObj != nullptr && SoundBase != nullptr)
	{
		VoiceAudioComponent->Sound = SoundBase;
		VoiceAudioComponent->Play();
	}

	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ACommanderCharacter::PlayAcknowledgeSound, TargetRecruit);
	GetWorldTimerManager().SetTimer(TargetRecruit.THandler_ResponseSound, RespawnDelegate, 1.0f, false, SoundBase->GetDuration());
}

void ACommanderCharacter::PlayAcknowledgeSound(FCommanderRecruit TargetRecruit)
{
	TargetRecruit.Recruit->getVoiceAudioComponent()->Sound = TargetRecruit.Recruit->getFactionObj()->getSelectedVoiceClipSet().AcknowledgeCommandSound;
	TargetRecruit.Recruit->getVoiceAudioComponent()->Play();
}

void ACommanderCharacter::ResetTargetActor()
{
	if (PotentialRecruit != nullptr)
	{
		PotentialRecruit->ShowCharacterOutline(false);
		PotentialRecruit = nullptr;
	}
}