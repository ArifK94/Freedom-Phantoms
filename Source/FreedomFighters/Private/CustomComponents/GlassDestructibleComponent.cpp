#include "CustomComponents/GlassDestructibleComponent.h"

void UGlassDestructibleComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UGlassDestructibleComponent::ReceiveComponentDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::ReceiveComponentDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("OnComponentDamage!"));
}