// Fill out your copyright notice in the Description page of Project Settings.


#include "Services/SharedService.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"

USharedService::USharedService()
{
}

bool USharedService::ThrowRotationAngle(FVector Start, FVector End, FRotator& TargetRotation)
{
	float Gravity = 980.f;

	// Velocity needs to be set as the same velocity for the projectile's movement component's initial speed. Alternatively, retrieve the projectile's intial speed dynamically.
	float Velocity = 1500.f;

	// X
	auto differenceXY = UKismetMathLibrary::VSize2D(FVector2D
	(
		End.X - Start.X,
		End.Y - Start.Y
	));


	// x^2
	auto SquaredX = UKismetMathLibrary::Square(differenceXY);

	// gravity * x^2
	auto SquaredGravity = SquaredX * Gravity;

	// velocity ^2
	float VelocityPower2 = UKismetMathLibrary::Square(Velocity);

	// velocity ^4
	float VelocityPower4 = UKismetMathLibrary::Square(VelocityPower2);

	// Z
	auto differenceZ = End.Z - Start.Z;

	// z * velocity ^2
	auto SquaredVeloZ = differenceZ * VelocityPower2;

	// 2 * z * velocity ^2
	auto DoubleSquaredVeloZ = SquaredVeloZ * 2.f;


	// gravity * (gravity * x^2 +  2 * z * v^2)
	auto TotalGravity = Gravity * (SquaredGravity + DoubleSquaredVeloZ);

	// sqrt eq
	auto SqrtEq = VelocityPower4 - TotalGravity;

	// Is Reachable?
	if (SqrtEq < 0.f) {
		return false;
	}

	// sqrt of eq
	auto SqrtOfEq = UKismetMathLibrary::Sqrt(SqrtEq);

	// gravity * x
	auto GravityX = differenceXY * Gravity;

	// velocity^2 + sqrt of eq
	auto SqrtGravityX = SqrtOfEq + VelocityPower2;

	auto FullEq = SqrtGravityX / GravityX;

	auto DegreesMax = UKismetMathLibrary::DegAtan(FullEq);


	// velocity^2 - sqrt of eq
	auto MinimalSqrtGravityX = VelocityPower2 - SqrtOfEq;

	auto FullEqMinimal = MinimalSqrtGravityX / GravityX;

	auto DegreesMin = UKismetMathLibrary::DegAtan(FullEqMinimal);

	auto MinTargetDegrees = UKismetMathLibrary::Min(DegreesMin, DegreesMax);


	auto LookAt = UKismetMathLibrary::FindLookAtRotation(Start, End);

	// Target rotation for the Projectile to make the curve throw.
	TargetRotation = UKismetMathLibrary::MakeRotator(LookAt.Roll, MinTargetDegrees, LookAt.Yaw);

	return true;
}

bool USharedService::IsTargetBehind(AActor* ActorA, AActor* TargetActor, float Amount)
{
	if (ActorA == nullptr || TargetActor == nullptr) {
		return false;
	}

	FVector MGForwardPos = UKismetMathLibrary::GetForwardVector(ActorA->GetActorRotation());
	FVector Normalised = TargetActor->GetActorLocation() - ActorA->GetActorLocation();
	UKismetMathLibrary::Vector_Normalize(Normalised);
	float Angle = UKismetMathLibrary::Dot_VectorVector(MGForwardPos, Normalised);

	if (Angle < Amount)
	{
		return true;
	}

	return false;
}

bool USharedService::IsNearTargetPosition(FVector Start, FVector Location, float Radius)
{
	// if zero, then this is assumed a target destination has not been set, therefore it is near target.
	if (Location.IsZero()) {
		return true;
	}


	FVector StartLocation = Start;
	StartLocation.Z = 0.f;

	FVector EndLocation = Location;
	EndLocation.Z = 0.f;

	if (UKismetMathLibrary::Vector_Distance(StartLocation, EndLocation) <= Radius) {
		return true;
	}

	return false;
}

bool USharedService::IsNearTargetPosition(AActor* ActorA, AActor* ActorB, float Radius)
{
	if (ActorA == nullptr || ActorB == nullptr) {
		return false;
	}

	return IsNearTargetPosition(ActorA->GetActorLocation(), ActorB->GetActorLocation(), Radius);
}

bool USharedService::CanSeeTarget(UWorld* World, FVector Start, AActor* TargetActor, AActor* Owner)
{
	if (World == nullptr || TargetActor == nullptr) {
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.bTraceComplex = true;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AllObjects;

	FVector EyeLocation;
	FRotator EyeRotation;
	TargetActor->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FHitResult OutHitTarget;
	bool bHitTarget = World->LineTraceSingleByObjectType(OutHitTarget, Start, EyeLocation, ObjectParams, QueryParams);

	return bHitTarget && OutHitTarget.GetActor() == TargetActor;
}

bool USharedService::IsInAir(FHitResult& OutHit, AActor* Actor, float Length)
{
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(Actor);

	FVector Start = Actor->GetActorLocation();
	FVector End = Start + FVector(.0f, .0f, -Length);

	return Actor->GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);
}

void USharedService::DestroyActorComponent(UActorComponent* ActorComponent)
{
	if (ActorComponent)
	{
		ActorComponent->DestroyComponent();
	}
}

bool USharedService::IsActorOnScreen(UObject* WorldContextObject, AActor* Actor, FVector2D Offset)
{
	auto PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0);

	FVector2D ScreenLocation;
	PlayerController->ProjectWorldLocationToScreen(Actor->GetActorLocation(), ScreenLocation);

	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(WorldContextObject);

	FVector2D ScreenLocOffset = ScreenLocation + Offset;

	// if all conditions are met. then actor is on the screen.
	return ScreenLocation.X > 0 && ScreenLocation.Y > 0 && ScreenLocOffset.X < ViewportSize.X && ScreenLocOffset.Y < ViewportSize.Y;
}