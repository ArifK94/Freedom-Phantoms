#pragma once

#include "CoreMinimal.h"
#include "DestructibleComponent.h"
#include "GlassDestructibleComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FREEDOMFIGHTERS_API UGlassDestructibleComponent : public UDestructibleComponent
{
	GENERATED_BODY()

private:
	virtual void BeginPlay() override;

	virtual void ReceiveComponentDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
};
