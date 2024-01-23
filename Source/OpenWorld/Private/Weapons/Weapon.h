// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraDataInterfaceExport.h"
#include "Weapon.generated.h"

class AOWCharacter;
class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * Implements that interface to make able to spawn blood decal 
 */
UCLASS()
class AWeapon : public AActor, public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	// ===== Interfaces ========== //

	void EquipTo(AOWCharacter* NewOwner, FName SocketName);
	void Drop();

	// ===== Combat ========== //

	void EnableCollision(bool bEnabled);
	void SetTempDamage(float TempDamage);
	virtual void ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> BloodTrailComponent;

	// ===== Events ========== //

	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// ===== Combat ========== //

	UPROPERTY()
	TWeakObjectPtr<AOWCharacter> CharacterOwner;

	UPROPERTY()
    TArray<AActor *> IgnoredActors;

	UPROPERTY(EditAnywhere, Category=Combat)
	float Damage = 20.f;

	float DefaultDamage = Damage;

	/** After certain time, set back the damage to default one */
	FTimerHandle TempDamageDelayHandler;

	void ApplyDamage(FHitResult &TraceResult);
    void HitTrace(FHitResult &TraceResult);

    // ===== VFX ========== //

    UPROPERTY(EditDefaultsOnly, Category = VFX)
    TSoftObjectPtr<UNiagaraSystem> BloodTrail;

	UPROPERTY(EditDefaultsOnly, Category=VFX)
	TSoftObjectPtr<UMaterialInterface> BloodSplatter;

public:
	// ===== Combat ========== //

	FORCEINLINE const float GetDamage() const
	{
		return Damage;
	}

private:
	void DefaultInitializer();
};
