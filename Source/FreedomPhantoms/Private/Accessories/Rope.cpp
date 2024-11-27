#include "Accessories/Rope.h"

#include "Components/SkeletalMeshComponent.h"

ARope::ARope()
{
	PrimaryActorTick.bCanEverTick = false;

	RopeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RopeMesh"));
	RopeMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	RootComponent = RopeMesh;

	IsRopeOccupied = false;
	IsRopeLeft = false;
}

void ARope::DeployRope()
{
	IsRopeDeployed = true;
}

void ARope::DetachRope()
{
	IsRopeDeployed = false;
	IsRopeReleased = true;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	SetLifeSpan(10.f);
}

void ARope::AttachActorToRope(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	IsRopeOccupied = true;
}

void ARope::DettachActorFromRope()
{
	IsRopeOccupied = false;
}

FVector ARope::GetBoneLocation(int32 BoneIndex) const
{
	if (RopeMesh && BoneIndex >= 0 && BoneIndex < RopeMesh->GetNumBones())
	{
		FName BoneName = RopeMesh->GetBoneName(BoneIndex);
		return RopeMesh->GetSocketLocation(BoneName);
	}
	return FVector::ZeroVector;
}

int32 ARope::GetNumBones() const
{
	return RopeMesh ? RopeMesh->GetNumBones() : 0;
}