#include "Animations/CharacterAnimNotify.h"

#include "Characters/CombatCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

void UCharacterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp == NULL || MeshComp->GetOwner() == NULL) {
		return;
	}

	ACombatCharacter* Character = Cast<ACombatCharacter>(MeshComp->GetOwner());

	if (!Character) {
		return;
	}

	if (MoveBackToCover) {

	}

	if (ShouldCrouch)
	{
		if (!Character->GetCharacterMovement()->IsCrouching())
			Character->ToggleCrouch();
	}

	if (AlignHandguardIK)
	{
		Character->SetHandGaurdIK(1.0f);
	}

	if (StopAlignHandguardIK)
	{
		Character->SetHandGaurdIK(0.0f);
	}

	if (IsPostDeath)
	{
		Character->PostDeath();
	}

	if (IsRevived)
	{
		Character->SetIsReviving(false);
	}

	if (EndCombat) {
		Character->EndAim();
		Character->EndFire();
	}

}
