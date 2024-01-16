// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class AOWCharacter;
class UBoxComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	// ===== Interfaces ========== //

	void EquipTo(AOWCharacter* NewOwner, FName SocketName);

	// ===== Combat ========== //

	void EnableCollision(bool bEnabled);
	

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
	TArray<AActor*> IgnoredActors;

	void HitTrace(FHitResult &TraceResult);

	// ===== VFX ========== //

	UPROPERTY(EditDefaultsOnly, Category=VFX)
	TSoftObjectPtr<UNiagaraSystem> BloodTrail;

private:
	void DefaultInitializer();
};
