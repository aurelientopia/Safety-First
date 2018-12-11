// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SafetyFirstProjectile.h"
#include "Components/BoxComponent.h"
#include "SafetyFirstWeapon.generated.h"



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

	UPROPERTY(EditAnywhere, meta = (Category = "Safety First ", DisplayName = "projectile class"))
	TSubclassOf<ASafetyFirstProjectile> m_ProjectileClass;

	/** scene root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* m_TriggerPickupComponent;

	int32 m_iNbMaxBullets = 0;


	int32 m_iNbBullet = 0;
	

	bool m_bRecoiling = false;

	FVector m_vRecoilDirection;

	float m_fRecoilTimeLeft = 0.0f;

	bool m_bCanBePickedUp = false;

	TWeakObjectPtr<AActor> m_WeaponOwner; 

public :
	/** Sound to play each time we fire */
	UPROPERTY(Category = Audio, EditAnywhere, BlueprintReadWrite)
	class USoundBase* m_FireSound;

	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
	float RecoilPower = 5000.0f;

	UPROPERTY(Category = Recoil, EditAnywhere, BlueprintReadOnly)
	UCurveFloat* m_RecoilDynamic = nullptr;
	
	UPROPERTY(Category = Recoil, EditAnywhere, BlueprintReadOnly)
	float m_fRecoilDistance = 1000.0f;

	UPROPERTY(Category = Recoil, EditAnywhere, BlueprintReadOnly)
	float m_fRecoilDuration = 1.0f;

	UPROPERTY(Category = Recoil, EditAnywhere, BlueprintReadOnly)
	float m_fDurationAfterWhichWeCanPickUpWeapon = 0.8f;
	
public:
	ASafetyFirstWeapon();

	/** Function to handle the projectile hitting something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	void Tick(float _fDt) override;

	/* Fire a shot in the specified direction */
	bool/*bEject*/ FireShot(FVector _vFireDirection);

	void RecoilLauncher(FVector _vFireDirection);

	float GetRecoilPower() { return RecoilPower; }
	bool CanBePickedUp() { return m_bCanBePickedUp; }

	void SetWeaponOwner(AActor* _weaponOwner) { m_WeaponOwner = _weaponOwner; }
	AActor* GetWeaponOwner() { return m_WeaponOwner.Get(); }
	
};

