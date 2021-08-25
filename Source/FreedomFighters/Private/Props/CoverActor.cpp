

#include "Props/CoverActor.h"
//#include "Characters/BaseCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

// Sets default values
ACoverActor::ACoverActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Init. components
	SM = CreateDefaultSubobject<UStaticMeshComponent>(FName("SM"));
	BoxComp = CreateDefaultSubobject<UBoxComponent>(FName("BoxComp"));
	BoxComp->SetCollisionProfileName(TEXT("Cover"));

	SetRootComponent(SM);

	BoxComp->SetupAttachment(SM);
}

// Called when the game starts or when spawned
void ACoverActor::BeginPlay()
{
	Super::BeginPlay();

	//Register overlap events
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ACoverActor::OnCompBeginOverlap);
	BoxComp->OnComponentEndOverlap.AddDynamic(this, &ACoverActor::OnCompEndOverlap);
}



void ACoverActor::OnCompBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void ACoverActor::OnCompEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}
