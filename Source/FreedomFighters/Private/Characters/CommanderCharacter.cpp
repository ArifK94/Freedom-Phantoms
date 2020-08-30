// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CommanderCharacter.h"
#include "..\..\Public\Characters\CommanderCharacter.h"


#include "Engine.h"


ACommanderCharacter::ACommanderCharacter()
{

}


void ACommanderCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	auto MyLocation = this->GetActorLocation();
	FHitResult OutHit;
	FVector Start = this->GetActorLocation();

	// alternatively you can get the camera location
	// FVector Start = FirstPersonCameraComponent->GetComponentLocation();

	FVector ForwardVector = FollowCamera->GetForwardVector();
	FVector End = ((ForwardVector * 1000.f) + Start);
	FCollisionQueryParams CollisionParams;

	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *OutHit.GetActor()->GetName()));
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Impact Point: %s"), *OutHit.ImpactPoint.ToString()));
			GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Normal Point: %s"), *OutHit.ImpactNormal.ToString()));
		}
	}
}

void ACommanderCharacter::Recruit(ACombatCharacter* Character)
{

}
