#include "Accessories/Rope.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"

#include "PhysicsEngine/PhysicsAsset.h"

ARope::ARope()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RopeMeshComp"));

	IsRopeOccupied = false;
	IsRopeLeft = false;

	TopAttachPointSocket = "AttachPoint_Top";
}

void ARope::BeginPlay()
{
	Super::BeginPlay();
}

void ARope::DropRope()
{
	//MeshComponent->PhysicsAssetOverride = RopeDropPhysics;
	//MeshComponent->SetSimulatePhysics(true);
	//SetLifeSpan(5);

	IsRopeDropped = true;
}


void ARope::ReleaseRope()
{
	IsRopeDropped = false;
	IsRopeReleased = true;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	GetOwner()->GetWorldTimerManager().SetTimer(THandler_Destroy, this, &ARope::DestroyRope, 1.f, false, 10.f);
}

void ARope::DestroyRope()
{
	Destroy();
}

void ARope::AttachActorToRope(AActor* Actor)
{
	Actor->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TopAttachPointSocket);
}