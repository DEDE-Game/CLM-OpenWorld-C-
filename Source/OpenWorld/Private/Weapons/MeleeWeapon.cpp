// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/MeleeWeapon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Enums/CollisionChannel.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AMeleeWeapon::AMeleeWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// Base Mesh
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
	BaseMesh->SetMobility(EComponentMobility::Movable);
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetRootComponent(BaseMesh);

	// Hit Box
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hit Box"));
	HitBox->SetupAttachment(RootComponent);
	HitBox->SetMobility(EComponentMobility::Movable);
	/** Collision */
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); /* Disable Meleeweapon collision at first */
	HitBox->SetGenerateOverlapEvents(true);
	HitBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

	// Interact Sphere
	InteractArea = CreateDefaultSubobject<USphereComponent>(TEXT("Interact Area"));
	InteractArea->SetupAttachment(RootComponent);
	InteractArea->SetMobility(EComponentMobility::Movable);
	/** Collision */
	InteractArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractArea->SetGenerateOverlapEvents(true);
	InteractArea->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	InteractArea->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// ...
	DefaultInitializer();
}

void AMeleeWeapon::DefaultInitializer()
{
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> BloodTrailAsset(
		TEXT("/Script/Niagara.NiagaraSystem'/Game/Game/VFX/Combat/NS_BloodTrail.NS_BloodTrail'")
	);
	BloodTrail = BloodTrailAsset.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BloodSplatterAsset(
		TEXT("/Script/Engine.MaterialInstanceConstant'/Game/Game/Materials/VFX/MI_BloodSplatter.MI_BloodSplatter'")
	);
	BloodSplatter = BloodSplatterAsset.Object;
}

// ==================== Lifecycles ==================== //

void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	// Binding Collision Events
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnWeaponOverlap);
	InteractArea->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnEnterInteract);
	InteractArea->OnComponentEndOverlap  .AddDynamic(this, &ThisClass::OnLeaveInteract);
}

// ==================== Collision Events ==================== //

void AMeleeWeapon::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner()) return;

	FHitResult TraceResult;
    HitTrace(TraceResult);
    ApplyDamage(TraceResult);
}

void AMeleeWeapon::OnEnterInteract(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

	if (!PlayerCharacter) return;

	PlayerCharacter->SetOverlappingWeapon(this);
	PlayerCharacter->ShowTip(TEXT("[F] - To pickup weapon"));
}

void AMeleeWeapon::OnLeaveInteract(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

	if (!PlayerCharacter) return;

	PlayerCharacter->SetOverlappingWeapon(nullptr);
	PlayerCharacter->HideTip();
}

// ==================== Interfaces ==================== //

void AMeleeWeapon::Pickup(AOWCharacter* NewOwner, FName SocketName)
{
	CharacterOwner = NewOwner;
	AttachedSocket = SocketName;
	SetOwner(NewOwner);
	EquipTo(false);

	// Disable interaction
	InteractArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMeleeWeapon::EquipTo(bool bEquipping)
{
	if (!CharacterOwner.IsValid()) return;
	
	FName SocketName = bEquipping ? TEXT("Hand Socket") : AttachedSocket;
	AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
}

void AMeleeWeapon::Drop()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// Enable physics
	BaseMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	BaseMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	BaseMesh->SetSimulatePhysics(true);

	// For the sake of performance, disable physics
	FTimerHandle DisableDelay;
	GetWorldTimerManager().SetTimer(DisableDelay, [this]() {
		BaseMesh->SetSimulatePhysics(false);
		BaseMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}, 9.f, false);

	// Reset Owner
	CharacterOwner = nullptr;
	SetOwner(nullptr);

	// Enable interaction
	InteractArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// ==================== Combat ==================== //

void AMeleeWeapon::ApplyDamage(FHitResult &TraceResult)
{
	IHitInterface* ActorHit = Cast<IHitInterface>(TraceResult.GetActor());

	// Only applying damage if other is enemy and damagable
	if (!ActorHit || !ActorHit->IsEnemy(CharacterOwner.Get())) return;
	
	// Apply damage
	float RandomOffset = FMath::RandRange(1.f, 5.f); 
	float GivenDamage  = FMath::RandRange(Damage - RandomOffset, Damage + RandomOffset);

	ActorHit->OnWeaponHit(CharacterOwner.Get(), TraceResult.ImpactPoint, GivenDamage, bBlockable);

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

void AMeleeWeapon::HitTrace(FHitResult& TraceResult)
{
    FVector Offset = HitBox->GetUpVector() * HitBox->GetScaledBoxExtent().Z;
    FVector StartTrace = HitBox->GetComponentLocation() - Offset;
    FVector EndTrace   = HitBox->GetComponentLocation() + Offset;

	IgnoredActors.AddUnique(GetOwner());

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoredActors);
	QueryParams.bTraceComplex = false;

	GetWorld()->SweepSingleByChannel(
		TraceResult,
		StartTrace,
		EndTrace,
		GetActorRotation().Quaternion(),
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeBox(HitBox->GetScaledBoxExtent()),
		QueryParams
	);

	// If its already hit that actor
	if (TraceResult.bBlockingHit)
		IgnoredActors.AddUnique(TraceResult.GetActor());
}

void AMeleeWeapon::EnableCollision(bool bEnabled)
{
	HitBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

	// Reset
	if (!bEnabled)
	{
		IgnoredActors.Empty();
	}
}

void AMeleeWeapon::SetTempDamage(float TempDamage, bool bDamageBlockable /*= true*/)
{
	// Update it now
	Damage 	   = TempDamage;
	bBlockable = bDamageBlockable;

	// Set the timer
	GetWorldTimerManager().SetTimer(TempDamageDelayHandler, [this]() {
		Damage 	   = DefaultDamage;
		bBlockable = true;
	}, 1.f, false);
}

void AMeleeWeapon::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset)
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
