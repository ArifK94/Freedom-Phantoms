#include "CustomComponents/CoverSplineComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/ArrowComponent.h"

UCoverSplineComponent::UCoverSplineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	//MC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	//MC->RegisterComponent();
	//MC->Activate();


	//UArrowComponent* ArrowDirection = NewObject<UArrowComponent>(this, UArrowComponent::StaticClass());
	//ArrowDirection->SetWorldLocation(StartPoint);
	//ArrowDirection->SetWorldRotation();
	//ArrowDirection->SetCollisionProfileName(TEXT("OverlapAll"));
	//ArrowDirection->OnComponentBeginOverlap.AddDynamic(this, &UCoverSplineComponent::OnCoverBeginOverlap);
	//ArrowDirection->AttachTo(RootComponent);
}


void UCoverSplineComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UCoverSplineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCoverSplineComponent::OnCoverBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((OtherActor != NULL)  && (OtherComp != NULL))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("This is an on screen message from Spline!"));
	}
}