#include "Animations/CharacterAnimNotify.h"

#include "Characters/CombatCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

void UCharacterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		ACombatCharacter* Character = Cast<ACombatCharacter>(MeshComp->GetOwner());

		if (Character)
		{
			if (MoveBackToCover) {
				Character->MoveToCover();
			}

			if (ShouldCrouch)
			{
				if (!Character->GetCharacterMovement()->IsCrouching())
					Character->BeginCrouch();
			}

			if (AlignHandguardIK)
			{
				Character->SetHandGaurdIK(1.0f);
			}

			if (StopAlignHandguardIK)
			{
				Character->SetHandGaurdIK(0.0f);
			}
		}
	}
}
