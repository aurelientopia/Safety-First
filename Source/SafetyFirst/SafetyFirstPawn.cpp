// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SafetyFirstPawn.h"
#include "SafetyFirstProjectile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

const FName ASafetyFirstPawn::MoveForwardBinding("MoveForward");
const FName ASafetyFirstPawn::MoveRightBinding("MoveRight");
const FName ASafetyFirstPawn::FireForwardBinding("FireForward");
const FName ASafetyFirstPawn::FireRightBinding("FireRight");
const FName ASafetyFirstPawn::FireBinding("Fire");



ASafetyFirstPawn::ASafetyFirstPawn()
{	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/TwinStick/Meshes/TwinStickUFO.TwinStickUFO"));
	// Create the mesh component
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);
	

	// Create the fire Direction component
	FireDirComponent = CreateDefaultSubobject<USceneComponent>(TEXT("FireDir"));
	

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FireDirMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cone.Shape_Cone"));
	// Create the fire direction mesh component
	FireDirMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FireDirMesh"));
	FireDirMeshComponent->SetupAttachment(FireDirComponent);
	FireDirMeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	FireDirMeshComponent->SetStaticMesh(FireDirMesh.Object);


	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/TwinStick/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1200.f;
	CameraBoom->RelativeRotation = FRotator(-80.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;


}

void ASafetyFirstPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveRightBinding);
	PlayerInputComponent->BindAxis(FireForwardBinding);
	PlayerInputComponent->BindAxis(FireRightBinding);
	PlayerInputComponent->BindAxis(FireBinding);
}


void ASafetyFirstPawn::BeginPlay()
{
	Super::BeginPlay();
	if (m_WeaponClass != nullptr)
	{
		FActorSpawnParameters spawnInfo;
		spawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASafetyFirstWeapon* weapon = GetWorld()->SpawnActor<ASafetyFirstWeapon>(m_WeaponClass, GetActorTransform(), spawnInfo);
		if (weapon != nullptr)
		{
			RetrieveWeapon(weapon);
		}
	}
	
}

void ASafetyFirstPawn::Tick(float DeltaSeconds)
{
	//UE_LOG(LogTemp, Warning, TEXT("test "));
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * DeltaSeconds;

	// If non-zero size, move this actor
	if (Movement.SizeSquared() > 0.0f)
	{
		const FRotator NewRotation = Movement.Rotation();
		FHitResult Hit(1.f);
		RootComponent->MoveComponent(Movement, NewRotation, true, &Hit);
		
		if (Hit.IsValidBlockingHit())
		{
			const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
			const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
			RootComponent->MoveComponent(Deflection, NewRotation, true);
		}
	}
	
	// Create fire direction vector
	const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	const float FireRightValue = GetInputAxisValue(FireRightBinding);
	if (FVector(FireForwardValue, FireRightValue, 0.f).SizeSquared() > 0.0f)
	{
		m_vFireDirection = FVector(FireForwardValue, FireRightValue, 0.f);
	}
	
	FRotator FireDirRotator = m_vFireDirection.Rotation();

	FireDirComponent->SetWorldRotation(FireDirRotator);

	bool ShootButtonPressed = false;
	if (GetInputAxisValue(FireBinding) > 0.0f)
	{
		ShootButtonPressed = true;
		// Try and fire a shot
		FireShot(m_vFireDirection);
	}	
}

void ASafetyFirstPawn::FireShot(FVector FireDirection)
{
	// If it's ok to fire again
	if (bCanFire == true)
	{
		// If we are pressing fire stick in a direction
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			// Spawn projectile at an offset from this pawn
			const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);

			UWorld* const World = GetWorld();
			if (World != NULL)
			{
				// spawn the projectile
				World->SpawnActor<ASafetyFirstProjectile>(SpawnLocation, FireRotation);
			}

			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &ASafetyFirstPawn::ShotTimerExpired, FireRate);

			// try and play the sound if specified
			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}

			bCanFire = false;
		}
	}
}

void ASafetyFirstPawn::ShotTimerExpired()
{
	bCanFire = true;
}

void ASafetyFirstPawn::RetrieveWeapon(ASafetyFirstWeapon* _weapon)
{
	m_Weapon = _weapon;

	FAttachmentTransformRules transformRules(EAttachmentRule::SnapToTarget, /*bInWeldSimulatedBodies*/false);
	m_Weapon->AttachToActor(this, transformRules);
}
