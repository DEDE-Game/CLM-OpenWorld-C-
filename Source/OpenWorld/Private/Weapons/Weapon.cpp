// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/Weapon.h"
#include "Components/BoxComponent.h"
#include "Characters/OWCharacter.h"
#include "Enums/CollisionChannel.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Weapon.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// Base Mesh
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
	BaseMesh->SetMobility(EComponentMobility::Movable);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetRootComponent(BaseMesh);

	// Collision Box
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetMobility(EComponentMobility::Movable);
	/** Collision */
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); /* Disable weapon collision at first */
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// ...
	DefaultInitializer();
}

void AWeapon::DefaultInitializer()
{
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BloodTrailAsset(
		TEXT("/Script/Niagara.NiagaraSystem'/Game/Game/VFX/Combat/NS_BloodTrail.NS_BloodTrail'")
	);
	BloodTrail = BloodTrailAsset.Object;
}

// ==================== Lifecycles ==================== //

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	// Binding Events
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnWeaponOverlap);
}

// ==================== Events ==================== //

void AWeapon::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner()) return;

	FHitResult TraceResult;
    HitTrace(TraceResult);
    ApplyDamage(TraceResult);
}

void AWeapon::ApplyDamage(FHitResult &TraceResult)
{
	IHitInterface* ActorHit = Cast<IHitInterface>(TraceResult.GetActor());

	// Only applying damage if other is enemy and damagable
	if (!ActorHit || !ActorHit->IsEnemy(CharacterOwner.Get())) return;
	
	// Apply damage
	float RandomOffset = FMath::RandRange(1.f, 5.f); 
	float GivenDamage  = FMath::RandRange(Damage - RandomOffset, Damage + RandomOffset);

	ActorHit->OnWeaponHit(CharacterOwner.Get(), TraceResult.ImpactPoint, GivenDamage);

	// Spawn blood trail only when oponent is not blocking the attack
	if (!ActorHit->IsBlocking())
	{
		BloodTrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			BloodTrail.LoadSynchronous(),
			BaseMesh,
			TEXT("EndSocket"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true);
		BloodTrailComponent->Activate();
		BloodTrailComponent->SetVariableObject(TEXT("User.ObjCollisionCallback"), this);
	}
}

void AWeapon::HitTrace(FHitResult& TraceResult)
{
    FVector Offset = CollisionBox->GetUpVector() * CollisionBox->GetScaledBoxExtent().Z;
    FVector StartTrace = CollisionBox->GetComponentLocation() - Offset;
    FVector EndTrace   = CollisionBox->GetComponentLocation() + Offset;

	IgnoredActors.AddUnique(GetOwner());

    UKismetSystemLibrary::BoxTraceSingle(
        this,
        StartTrace,
        EndTrace,
        CollisionBox->GetScaledBoxExtent(),
        GetActorRotation(),
        ETraceTypeQuery::TraceTypeQuery1,
        false,
        IgnoredActors,
        EDrawDebugTrace::None,
        TraceResult,
        true
	);

	// If its already hit that actor
	if (TraceResult.bBlockingHit)
		IgnoredActors.AddUnique(TraceResult.GetActor());
}

// ==================== Interfaces ==================== //

void AWeapon::EquipTo(AOWCharacter* NewOwner, FName SocketName)
{
	CharacterOwner = NewOwner;

	SetOwner(NewOwner);
	AttachToComponent(NewOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
}

void AWeapon::Drop()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	BaseMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	BaseMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	BaseMesh->SetSimulatePhysics(true);
}

// ==================== Combat ==================== //

void AWeapon::EnableCollision(bool bEnabled)
{
	CollisionBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

	// Reset
	if (!bEnabled)
	{
		IgnoredActors.Empty();
	}
}

void AWeapon::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset)
{
	// Spawn a blood splatter
	for (const auto& Dat : Data)
	{
		UGameplayStatics::SpawnDecalAtLocation(
			this,
			BloodSplatter.LoadSynchronous(),
			{ 5.f, 10.f, 10.f },
			Dat.Position,
			FRotator(-90.f, 0.f, 0.f),
			5.f
		);
	}
}
