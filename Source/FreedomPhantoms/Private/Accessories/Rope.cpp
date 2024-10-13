#include "Accessories/Rope.h"

#include "CableComponent.h"


ARope::ARope()
{
	PrimaryActorTick.bCanEverTick = false;

	CableComp = CreateDefaultSubobject<UCableComponent>(TEXT("CableComponent"));

	// Attach the CableComp to the character's root component (or any other component, like a socket)
	CableComp->SetupAttachment(RootComponent);

	// Optional settings for the cable
	CableComp->CableLength = 500.0f;            // Length of the cable
	CableComp->NumSegments = 20;                // Number of segments in the cable for smoothness
	CableComp->SolverIterations = 8;            // Physics solver iterations for accuracy
	CableComp->CableWidth = 5.0f;               // Visual width of the cable
	CableComp->bEnableCollision = true;         // Enable collision on the cable
	CableComp->SetVisibility(true);             // Make cable visible

	IsRopeOccupied = false;
	IsRopeLeft = false;
}

void ARope::BeginPlay()
{
	Super::BeginPlay();

	UpdateCableLength();
}

void ARope::UpdateCableLength()
{
	// Get the helicopter's location
	FVector MyLocation = GetActorLocation();

	// Set the trace distance downwards (e.g., 10,000 units)
	FVector EndLocation = MyLocation - FVector(0, 0, 100000.f);

	// Perform a line trace to detect the ground
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	if (GetOwner()) 
	{
		CollisionParams.AddIgnoredActor(GetOwner());
	}

	if (GetAttachParentActor())
	{
		CollisionParams.AddIgnoredActor(GetAttachParentActor());
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, MyLocation, EndLocation, ECC_Visibility, CollisionParams);

	if (bHit)
	{
		if (HitResult.GetActor())
		{
			CableComp->SetAttachEndTo(HitResult.GetActor(), NAME_None);
			CableComp->EndLocation = HitResult.Location;
		}

		// Calculate the distance between the helicopter and the ground
		float Distance = (MyLocation - HitResult.Location).Size();

		// Adjust the cable length based on the distance
		CableComp->CableLength = Distance;
	}
}


void ARope::DropRope()
{
	IsRopeDropped = true;
}

void ARope::ReleaseRope()
{
	IsRopeDropped = false;
	IsRopeReleased = true;
	
	CableComp->bAttachStart = false;
	CableComp->bAttachEnd = false;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetLifeSpan(10.f);
}

void ARope::AttachActorToRope(AActor* Actor)
{
	Actor->AttachToComponent(CableComp, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), FName("CableStartSocket"));

	IsRopeOccupied = true;
}

void ARope::DettachActorToRope()
{
	IsRopeOccupied = false;
}

FVector ARope::GetStartLocation()
{
	return CableComp->GetComponentLocation();
}

FVector ARope::GetEndLocation()
{
	// Get the start location of the cable (in world space)
	FVector CableStartLocation = CableComp->GetComponentLocation();

	// Calculate the end location using the CableLength
	CableStartLocation.Z = CableStartLocation.Z - CableComp->CableLength;

	return CableStartLocation;
}