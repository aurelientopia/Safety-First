// Copyright 1998-2018 Epic Games, Inc. All Rights Reserve

#include "SafetyFirstWeapon.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "SafetyFirstProjectile.h"
#include "Kismet/GameplayStatics.h"

ASafetyFirstWeapon::ASafetyFirstWeapon()
{
	
	// Create mesh component for the projectile sphere
	m_RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRootComponent"));
	m_RootSceneComponent->SetupAttachment(RootComponent);
	RootComponent = m_RootSceneComponent;


	m_FirePositionStartComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Fire position"));
	m_FirePositionStartComponent->SetupAttachment(RootComponent);

	m_TriggerPickupComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger pickup"));
	m_TriggerPickupComponent->SetupAttachment(RootComponent);
	m_TriggerPickupComponent->SetCollisionProfileName("NoCollision");


	/*
	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
	*/

	PrimaryActorTick.bCanEverTick = true;
}

void ASafetyFirstWeapon::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics())
	{
		OtherComp->AddImpulseAtLocation(GetVelocity() * 20.0f, GetActorLocation());
	}

	Destroy();
}


void ASafetyFirstWeapon::Tick(float _fDt)
{
	if (m_bRecoiling)
	{

		float fCurRecoilRatio = (m_fRecoilDuration - m_fRecoilTimeLeft) / m_fRecoilDuration;
		fCurRecoilRatio = FMath::Clamp(fCurRecoilRatio, 0.0f, 1.0f);
		m_fRecoilTimeLeft -= _fDt;
		float fNextRecoilRatio = (m_fRecoilDuration - m_fRecoilTimeLeft) / m_fRecoilDuration;
		fNextRecoilRatio = FMath::Clamp(fNextRecoilRatio, 0.0f, 1.0f);

		if (FMath::IsNearlyEqual(fNextRecoilRatio, 1.0f))
		{
			m_bRecoiling = false;
		}

		if (m_RecoilDynamic != nullptr)
		{
			fCurRecoilRatio = FMath::Clamp(m_RecoilDynamic->GetFloatValue(fCurRecoilRatio), 0.0f, 1.0f);
			fNextRecoilRatio = FMath::Clamp(m_RecoilDynamic->GetFloatValue(fNextRecoilRatio), 0.0f, 1.0f);
		}


		FVector vFrameMovement = ((fNextRecoilRatio - fCurRecoilRatio) * m_fRecoilDistance) * m_vRecoilDirection;

		SetActorLocation(GetActorLocation() + vFrameMovement);

		FRotator currentRotation = GetActorRotation();
		currentRotation.Yaw += m_fRecoilRotationRatePerSec * _fDt;
		SetActorRotation(currentRotation);


		if (!m_bCanBePickedUp && m_fRecoilDuration - m_fRecoilTimeLeft > m_fDurationAfterWhichWeCanPickUpWeapon)
		{
			m_bCanBePickedUp = true;
		}
	}
}

bool ASafetyFirstWeapon::FireShot(FVector _vFireDirection)
{
	bool bEject = false;
	// If we are pressing fire stick in a direction
	if (_vFireDirection.SizeSquared() > 0.0f)
	{
		const FRotator FireRotation = _vFireDirection.Rotation();
		// Spawn projectile at an offset from this pawn
		const FVector vSpawnLocation = m_FirePositionStartComponent->GetComponentLocation();

		UWorld* const World = GetWorld();
		if (World != NULL && m_ProjectileClass != nullptr)
		{
			// spawn the projectile
			World->SpawnActor<ASafetyFirstProjectile>(m_ProjectileClass, vSpawnLocation, FireRotation);
		}

		// try and play the sound if specified
		if (m_FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, m_FireSound, GetActorLocation());
		}

		bEject = true;
	}


	return bEject;
}

void ASafetyFirstWeapon::RecoilLauncher(FVector _vFireDirection)
{
	m_bRecoiling = true;
	m_vRecoilDirection = (_vFireDirection * (-1.0f)).GetSafeNormal2D();
	m_fRecoilTimeLeft = m_fRecoilDuration;
	m_bCanBePickedUp = false;
	m_TriggerPickupComponent->SetCollisionProfileName("Trigger");
}
