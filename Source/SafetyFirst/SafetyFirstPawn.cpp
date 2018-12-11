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
const FName ASafetyFirstPawn::PickUpBinding("PickUp");


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
	PlayerInputComponent->BindAction(PickUpBinding, IE_Pressed, this, &ASafetyFirstPawn::PickUpPressed);
	PlayerInputComponent->BindAction(PickUpBinding, IE_Released, this, &ASafetyFirstPawn::PickUpReleased);
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

	m_vFireDirection = GetActorForwardVector();
	
}


void ASafetyFirstPawn::Tick(float _fDt)
{
	Super::Tick(_fDt);

	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float RightValue = GetInputAxisValue(MoveRightBinding);

	
	
	// Create fire direction vector
	const float FireForwardValue = GetInputAxisValue(FireForwardBinding);
	const float FireRightValue = GetInputAxisValue(FireRightBinding);
	if (FVector(FireForwardValue, FireRightValue, 0.f).SizeSquared() > m_fDeadZoneRightStick * m_fDeadZoneRightStick)
	{
		m_vFireDirection = FVector(FireForwardValue, FireRightValue, 0.f).GetSafeNormal2D();
	}
	
	FRotator FireDirRotator = m_vFireDirection.Rotation();

	FireDirComponent->SetWorldRotation(FireDirRotator);

	
	if (m_Weapon.IsValid())
	{
		if (GetInputAxisValue(FireBinding) > 0.0f)
		{
			if (!m_bHasFirePressed)
			{
				m_bHasFirePressed = true;
				bool bWeaponEjected = m_Weapon->FireShot(m_vFireDirection);
				if (bWeaponEjected)
				{
					FDetachmentTransformRules detachmentRules(/*InLocationRule*/EDetachmentRule::KeepWorld, /*InRotationRule*/EDetachmentRule::KeepWorld, /*InScaleRule*/EDetachmentRule::KeepWorld, /*bInCallModify*/true);
					m_Weapon->DetachFromActor(detachmentRules);
					m_Weapon->SetWeaponOwner(nullptr);
					m_Weapon->RecoilLauncher(m_vFireDirection);
					m_Weapon = nullptr;
				}
			}
		}
		else
		{
			m_bHasFirePressed = false;
		}
	}	

	// Clamp max size so that (X=1, Y=1) doesn't cause faster movement in diagonal directions
	const FVector MoveDirection = FVector(ForwardValue, RightValue, 0.f).GetClampedToMaxSize(1.0f);

	// Calculate  movement
	const FVector Movement = MoveDirection * MoveSpeed * _fDt;

	// If non-zero size, move this actor
	FHitResult Hit(1.f);
	RootComponent->MoveComponent(Movement, FireDirRotator, true, &Hit);

	if (Hit.IsValidBlockingHit())
	{
		const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
		const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
		RootComponent->MoveComponent(Deflection, FireDirRotator, true);
	}


	if (m_WeaponPickup.IsValid())
	{
		if (m_WeaponPickup->CanBePickedUp() && m_bWantPickup)
		{
			RetrieveWeapon(m_WeaponPickup.Get());
			m_WeaponPickup = nullptr;
			m_bWantPickup = false;
		}
		else if (m_WeaponPickup->GetWeaponOwner() != nullptr)
		{
			//it has been picked up already
			m_WeaponPickup = nullptr;
		}
	}


	if (m_bWantPickup)
	{
		m_fPickupLifeSpan -= _fDt;
		if (m_fPickupLifeSpan <= 0.0f)
		{
			m_bWantPickup = false;
		}
	}
	
}


void ASafetyFirstPawn::RetrieveWeapon(ASafetyFirstWeapon* _weapon)
{
	if (_weapon != nullptr)
	{
		m_Weapon = _weapon;
		m_Weapon->SetWeaponOwner(this);
		FAttachmentTransformRules transformRules(/*InLocationRule*/EAttachmentRule::KeepRelative, /*InRotationRule*/EAttachmentRule::SnapToTarget, /*InScaleRule*/EAttachmentRule::KeepWorld, /*bInWeldSimulatedBodies*/false);
		m_Weapon->AttachToActor(this, transformRules);
		m_Weapon->SetActorRelativeLocation(m_vWeaponAttachmentOffset);
	}
}

void ASafetyFirstPawn::PickUpPressed()
{
	if (!m_bPickupPressed)
	{
		m_bPickupPressed = true;
		m_bWantPickup = true;
		m_fPickupLifeSpan = m_fDurationOfPickupLifeSpan;
	}
}

void ASafetyFirstPawn::PickUpReleased()
{
	m_bPickupPressed = false;
	m_bWantPickup = true;
}

void ASafetyFirstPawn::NotifyActorBeginOverlap(AActor* _otherActor)
{
	Super::NotifyActorBeginOverlap(_otherActor);
	if (ASafetyFirstWeapon* weapon = Cast<ASafetyFirstWeapon>(_otherActor))
	{
		if (weapon->GetWeaponOwner() == nullptr)
		{
			m_WeaponPickup = weapon;
		}
	}
}


void ASafetyFirstPawn::NotifyActorEndOverlap(AActor* _otherActor)
{
	Super::NotifyActorEndOverlap(_otherActor);
	if (ASafetyFirstWeapon* weapon = Cast<ASafetyFirstWeapon>(_otherActor))
	{
		if (m_WeaponPickup.Get() == weapon)
		{
			m_WeaponPickup = nullptr;
		}
	}
}
