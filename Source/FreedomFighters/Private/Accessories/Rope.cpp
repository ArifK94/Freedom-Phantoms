#include "Accessories/Rope.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"

#include "PhysicsEngine/PhysicsAsset.h"

ARope::ARope()
{
	PrimaryActorTick.bCanEverTick = false;

	ropeMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RopeMeshComp"));
}

void ARope::BeginPlay()
{
	Super::BeginPlay();

	CreateCollisionPoints();
	ropeMeshComp->PhysicsAssetOverride = RopeRappelPhysics;
}


void ARope::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Some debug message!"));

}

void ARope::DropRope()
{
	ropeMeshComp->PhysicsAssetOverride = RopeDropPhysics;
	ropeMeshComp->SetSimulatePhysics(true);
}

void ARope::CreateCollisionPoints()
{
	const int32 TotalBones = ropeMeshComp->GetNumBones();

	for (int i = 0; i < TotalBones; i++)
	{
		//UCapsuleComponent* CollisionPoint = NewObject<UCapsuleComponent>(this, UCapsuleComponent::StaticClass());
		//CollisionPoint->SetWorldLocation(ropeMeshComp->GetBoneLocation(ropeMeshComp->GetBoneName(i), EBoneSpaces::WorldSpace));

		//CollisionPoint->AttachToComponent(ropeMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ropeMeshComp->GetBoneName(i));
		//CollisionPoint->CapsuleRadius(220.0f);

		//CollisionPoint->ShapeColor = FColor(0, 0, 255, 255);
		//CollisionPoint->OnComponentBeginOverlap.AddDynamic(this, &ARope::OnOverlapBegin);

		//CollisionPoint->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
		//CollisionPoint->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);


		const FVector StartPoint = ropeMeshComp->GetBoneLocation(ropeMeshComp->GetBoneName(i), EBoneSpaces::WorldSpace);

		UBoxComponent* CollisionBox = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass());
		CollisionBox->SetWorldLocation(StartPoint);
		CollisionBox->ShapeColor = FColor(0, 0, 255, 255);




	}
}