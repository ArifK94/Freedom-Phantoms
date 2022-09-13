#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoCrate.generated.h"

class UAudioComponent;
UCLASS()
class FREEDOMPHANTOMS_API AAmmoCrate : public AActor
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* ReplenishSuccessSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* ReplenishFailedSound;

public:	
	AAmmoCrate();

	void PlaySuccess();

	void PlayFailed();

private:
	virtual void BeginPlay() override;


	UFUNCTION()
		void OnCrateHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
