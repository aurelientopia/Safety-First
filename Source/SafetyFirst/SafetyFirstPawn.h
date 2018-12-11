// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SafetyFirstWeapon.h"

#include "SafetyFirstPawn.generated.h"

UCLASS(Blueprintable)
class ASafetyFirstPawn : public APawn
{
	GENERATED_BODY()

	/* The mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ShipMeshComponent;

	/* The fire dir */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* FireDirComponent;

	/* The fire dir mesh component */
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* FireDirMeshComponent;

	/** The camera */
	//UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//class UCameraComponent* CameraComponent;

	/** Camera boom positioning the camera above the character */
	//UPROPERTY(Category = Camera, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//class USpringArmComponent* CameraBoom;
	
public:
	ASafetyFirstPawn();

	/** Offset from the ships location to spawn projectiles */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite )
	FVector GunOffset;
	
	/* How fast the weapon will fire */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float FireRate;

	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MoveSpeed;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float MoveSpeedLerp = 0.2f;

	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* FireSound;

	UPROPERTY(EditAnywhere, meta = (Category ="Safety First ", DisplayName = "weapon class"))
	TSubclassOf<ASafetyFirstWeapon> m_WeaponClass;

	UPROPERTY(EditAnywhere, meta = (Category = "Safety First ", DisplayName = "weapon attachmentOffset"))
	FVector m_vWeaponAttachmentOffset = FVector(2.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, meta = (Category = "Safety First ", DisplayName = "deadZone right stick"))
	float m_fDeadZoneRightStick = 0.2f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Spawn")
	void BPE_Fire(ASafetyFirstWeapon* _WeaponLaunched, FVector _vFireDirection);


	// Begin Actor Interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End Actor Interface
	
	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName FireForwardBinding;
	static const FName FireRightBinding;
	static const FName FireBinding;

private:

	FVector m_vFireDirection;

	UPROPERTY()
	ASafetyFirstWeapon* m_Weapon;

	bool m_bHasFirePressed = false;

	FVector m_Movement;

public:
	/** Returns ShipMeshComponent subobject **/
	FORCEINLINE class UStaticMeshComponent* GetShipMeshComponent() const { return ShipMeshComponent; }
	/** Returns CameraComponent subobject **/
	//FORCEINLINE class UCameraComponent* GetCameraComponent() const { return CameraComponent; }
	/** Returns CameraBoom subobject **/
	//FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

private:

	void RetrieveWeapon(ASafetyFirstWeapon* _weapon);
};

