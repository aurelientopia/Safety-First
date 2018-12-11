// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SafetyFirstWeapon.generated.h"

class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS(config=Game)
class ASafetyFirstWeapon : public AActor
{
	GENERATED_BODY()

private:
	/** scene root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	USceneComponent* m_RootSceneComponent;

	/** scene root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	USceneComponent* m_FirePositionStartComponent;


	int32 m_iNbMaxBullets = 0;


	int32 m_iNbBullet = 0;
	
public :
	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* m_FireSound;

public:
	ASafetyFirstWeapon();

	/** Function to handle the projectile hitting something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	/* Fire a shot in the specified direction */
	void FireShot(FVector _vFireDirection);
};

