#include "Accessories/Rope.h"

#include "Components/SkeletalMeshComponent.h"

ARope::ARope()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RopeMeshComp"));
	RootComponent = MeshComponent;

	IsRopeOccupied = false;
	IsRopeLeft = false;

	TopAttachPointSocket = "AttachPoint_Top";
}


void ARope::DropRope()
{
	IsRopeDropped = true;
}


void ARope::ReleaseRope()
{
	IsRopeDropped = false;
	IsRopeReleased = true;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (GetWorld())
	{
		GetWorldTimerManager().SetTimer(THandler_Destroy, this, &ARope::DestroyRope, 1.f, false, 10.f);
	}
}

void ARope::DestroyRope()
{
	Destroy();
}

void ARope::AttachActorToRope(AActor* Actor)
{
	Actor->AttachToComponent(MeshComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TopAttachPointSocket);
	IsRopeOccupied = true;
}

void ARope::DettachActorToRope()
{
	IsRopeOccupied = false;
}