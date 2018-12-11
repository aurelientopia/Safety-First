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


	m_FirePositionStartComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Fire postition"));
	m_FirePositionStartComponent->SetupAttachment(RootComponent);

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
		if (World != NULL)
		{
			// spawn the projectile
			World->SpawnActor<ASafetyFirstProjectile>(vSpawnLocation, FireRotation);
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

}
