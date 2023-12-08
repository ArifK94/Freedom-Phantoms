// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterCinematic.generated.h"

class USkeletalMeshComponent;
UCLASS()
class FREEDOMPHANTOMS_API ACharacterCinematic : public AActor
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* BodyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LoaoutMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* PrimaryWeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* PrimaryWeaponScope;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HolsterWeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HolsterWeaponScope;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	FName WeaponHandSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	bool ShowPrimaryWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	bool ShowSecondaryWeapon;

	/**
	* Fix body transform to avoid manually constantly fix mesh transform in the world. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (AllowPrivateAccess = "true"))
	bool FixTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	FName SidearmHolsterSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	FName BackHolsterSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	FName Holster1Socket;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UseSidearmHolster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UseBackHolster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UseHolster1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UsePrimarySidearmHolster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UsePrimaryBackHolster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Holster", meta = (AllowPrivateAccess = "true"))
	bool UsePrimaryHolster;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	bool UseACOG;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	bool UseEOTECH;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	bool UseREDDOT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	bool UseTHERMAL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	FName ACOGSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	FName EOTECHSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	FName REDDOTSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	FName THERMALSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* ACOGMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* EOTECHMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* REDDOTMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default Weapon Attachments", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* THERMALMesh;
	
public:	
	ACharacterCinematic();

	UFUNCTION(BlueprintCallable)
	void SetWeaponHand(USkeletalMeshComponent* ParentWeapon, FName NewHandSocket);

private:
	void OnConstruction(const FTransform& Transform) override;


#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif


	void Update();

	void ToggleAttachments(USkeletalMeshComponent* ParentWeapon, USkeletalMeshComponent* ScopeMeshComp);

	void ChangeWeaponAttachment(USkeletalMeshComponent* ParentWeapon, USkeletalMeshComponent* ScopeMeshComp, USkeletalMesh* AttachmentMesh, FName Socket);

	void SetWeaponHolster(USkeletalMeshComponent* ParentWeapon, bool CanUseSidearmHolster, bool CanUseBackHolster, bool CanUseHolster1, bool HoldWeapon);

	void FixBodyTransform();
};
