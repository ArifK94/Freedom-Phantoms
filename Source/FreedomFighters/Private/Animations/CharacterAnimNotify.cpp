#include "Animations/CharacterAnimNotify.h"

#include "Characters/BaseCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

void UCharacterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp != NULL && MeshComp->GetOwner() != NULL)
	{
		ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());

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
		}
	}
}
