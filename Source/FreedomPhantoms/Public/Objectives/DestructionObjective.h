// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Objectives/BaseObjective.h"
#include "Interfaces/Interactable.h"
#include "DestructionObjective.generated.h"

/**
 * Objectives which involve destorying something, eg. destroying a bridge.
 */
UCLASS()
class FREEDOMPHANTOMS_API ADestructionObjective : public ABaseObjective, public IInteractable
{
	GENERATED_BODY()
	
private:
	FTimerHandle THandler_Countdown;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class URadialForceComponent* RadialForceComp;

	/** The countdown destruction timer. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float CountdownTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float CurrentCountdownTimer;

	/** Destruction damage applied to nearby health components */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		float DestructionDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* CountdownSound;

	/** Particle to play when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UParticleSystem* DestructionEffect;

	/** Sound to play when health reaches zero  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundBase* DestructionSound;

	/** Destruction sound attentuation  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		USoundAttenuation* DestructionAttenuation;

	/** Destruction actor, usually a C4 actor or something similar to be placed on the position of the interaction.  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<class AProjectile> DestructiveProjectileClass;

public:
	ADestructionObjective();

	// Interactable interface methods
	virtual FString GetKeyDisplayName_Implementation() override;
	virtual FString OnInteractionFound_Implementation(APawn* InPawn, AController* InController) override;
	virtual AActor* OnPickup_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool OnUseInteraction_Implementation(APawn* InPawn, AController* InController) override;
	virtual bool CanInteract_Implementation(APawn* InPawn, AController* InController) override;

	void OnDestruction();

private:
	void BeginCountdown();

	void ApplyExplosionDamage(FVector ImpactPoint);
};
