#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "CoverPointComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMFIGHTERS_API UCoverPointComponent : public USphereComponent
{
	GENERATED_BODY()

private:
	AActor* Owner;

	FVector Location;

	class UArrowComponent* ArrowComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsAtCornerLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsAtCornerRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanCrouch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool PreferCrouched;

public:	
	UCoverPointComponent();

public:	
	FVector GetLocation() {
		return Location;
	}

	AActor* GetOwner() {
		return Owner;
	}

	void SetOccupant(AActor* Actor) {
		Owner = Actor;
	}

	bool IsACornerLeft() {
		return IsAtCornerLeft;
	}

	bool IsACornerRight() {
		return IsAtCornerRight;
	}
	
	bool IsCrouchPreferred() {
		return PreferCrouched;
	}
};
