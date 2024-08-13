#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "CoverPointComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FREEDOMPHANTOMS_API UCoverPointComponent : public USphereComponent
{
	GENERATED_BODY()

private:

	UPROPERTY()
		AActor* Owner;

	FVector Location;

	UPROPERTY()
		class UArrowComponent* ArrowComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsAtCornerLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsAtCornerRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool CanCrouch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool PreferCrouched;

	/** Setting this point to priority allows AI to prioritise this point than other non priority points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		bool IsPriority;

public:	
	UCoverPointComponent();

private:

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void UpdateShapes();

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

	bool GetIsPriority() { return IsPriority; }
};
