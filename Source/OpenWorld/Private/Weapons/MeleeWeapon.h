// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraDataInterfaceExport.h"
#include "MeleeWeapon.generated.h"

class AOWCharacter;
class UBoxComponent;
class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UWidgetComponent;

/**
 * Implements that interface to make able to spawn blood decal 
 */
UCLASS()
class AMeleeWeapon : public AActor, public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()
	
public:	
	AMeleeWeapon();

	// ===== Interfaces ========== //

	void Pickup(AOWCharacter* NewOwner, FName SocketName);
	void EquipTo(bool bEquipping);
	void Drop();

	// ===== Combat ========== //

	void EnableCollision(bool bEnabled);
	void SetTempDamage(float TempDamage, bool bDamageBlockable = true);
	virtual void ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

	// ===== Components ========== //

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> HitBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> BloodTrailComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> InteractArea;

	// ===== Collision Events ========== //

	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEnterInteract(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnLeaveInteract(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ===== Attributes ========== //

	FName AttachedSocket;

	/** This can be usefull for playing different attacking animation for character */
	UPROPERTY(EditDefaultsOnly, Category=Attributes)
	FName WeaponName;

	// ===== Combat ========== //

	UPROPERTY()
	TWeakObjectPtr<AOWCharacter> CharacterOwner;

	UPROPERTY()
    TArray<AActor*> IgnoredActors;

	/** If its false, even thought the target combat is on blocking state he will still get damage */
	bool bBlockable = true;

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
	// ===== Acessors ========== //

	FORCEINLINE const float GetDamage() const
	{
		return Damage;
	}
	FORCEINLINE const FString GetWeaponName() const
	{
		return WeaponName.ToString();
	}

private:
	void DefaultInitializer();
};
