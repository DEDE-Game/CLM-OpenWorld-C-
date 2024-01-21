// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enums/Team.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "OWCharacter.generated.h"

class AWeapon;
class UNiagaraSystem;

UCLASS(Abstract)
class AOWCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AOWCharacter();

	// ===== Lifecycles ========== //

	virtual void Tick(float DeltaTime) override;

	// ===== Combat ========== //

	virtual void OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& ImpactPoint, const float GivenDamage) override;
	virtual const bool IsEnemy(AOWCharacter* Other) const override;
	virtual const bool IsBlocking() const override
	{
		return bSucceedBlocking;
	}

protected:
	// ===== Lifecycles ========== //

	virtual void BeginPlay() override;

	// ===== Locomotions ========== //

	UPROPERTY(EditAnywhere, Category=Locomotions)
	float SprintSpeed = 800.f;

	UPROPERTY(EditAnywhere, Category=Locomotions)
	float RunSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category=Locomotions)
	float WalkSpeed = 200.f;

	bool bCanMove = true;

	void ToggleWalk(bool bToggled);
	void ToggleSprint(bool bToggled);
	void ToggleCrouch(bool bTogged);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ToggleMovement(bool bToggled)
	{
		bCanMove = bToggled;
	}

	UFUNCTION(BlueprintCallable)
	void MoveForward();

	// ===== Attributes ========== //

	UPROPERTY(EditAnywhere, Category=Attributes)
	float MaxHealth = 100.f;

	float Health = MaxHealth;

	FORCEINLINE void SetHealth(float Offset)
	{
		Health = FMath::Clamp(Health + Offset, 0.f, MaxHealth);

		if (IsDead()) Die();
	}

	virtual void Die();

	// ===== Combat ========== //

	UPROPERTY(EditDefaultsOnly, Category=Combat)
	ETeam Team = ETeam::T_Neutral;

	UPROPERTY()
	TWeakObjectPtr<AWeapon> CarriedWeapon;

	UPROPERTY()
	TWeakObjectPtr<AOWCharacter> TargetCombat;

	bool bEquipWeapon = false;
	bool bSucceedBlocking = false;

	/** Combo */
	int8 AttackCount  = 0;

	/** Combo timer, when its over so do the combo */
	FTimerHandle ComboOverHandler;

	UPROPERTY(EditAnywhere, Category=Combat)
	float ComboOverTimer = 2.f;

	FORCEINLINE void ComboOver();

	/** Blocking opponent's attack */
	void ToggleBlock(bool bToggled);

	UFUNCTION(BlueprintCallable)
	void EnableWeapon(bool bEnabled);

	/** Changing which weapon to equip */
	UFUNCTION(BlueprintCallable)
	void AttachWeapon();

	/** About to change the weapon */
	void SwapWeapon(float Value);

	/** If the oponent is farer than CombatRadius, it will not lock on that opponent anymore */
	UPROPERTY(EditAnywhere, Category=Combat)
	float CombatRadius = 1500.f;

	/**
	 * So it will always facing the target
	 * 
	 * @param Target If the target is nullptr, then the character wont lock on anything (Clear Lock On)
	 */
	virtual void SetLockOn(AOWCharacter* Target);

	void LockOn(float DeltaTime);

	void HitReaction(const FVector& ImpactPoint);

	/** Just attack */
	virtual void Attack();

    // ===== Animations ========== //

    UPROPERTY(EditDefaultsOnly, Category=Animations)
	TMap<FName, TSoftObjectPtr<UAnimMontage>> Montages;

	// ===== Audio ========== //

	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TMap<FName, TSoftObjectPtr<USoundBase>> FootstepSounds;

	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TSoftObjectPtr<USoundBase> HitfleshSound;

	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TSoftObjectPtr<USoundBase> BlockingSound;

	UFUNCTION(BlueprintCallable)
	void PlayFootstepSound();

	// ===== VFX ========== //

	UPROPERTY(EditDefaultsOnly, Category=VFX)
	TSoftObjectPtr<UNiagaraSystem> BloodSplash;

private:
	void DefaultInitializer();

public:
	// ===== Accessors ========== //

	FORCEINLINE AOWCharacter* GetTargetCombat()
	{
		return TargetCombat.Get();
	}
	FORCEINLINE const bool IsEquippingWeapon() const
	{
		return bEquipWeapon;
	}
	FORCEINLINE const bool IsDead() const
	{
		return Health == 0.f;
	}
	FORCEINLINE ETeam GetTeam() const 
	{
		return Team;
	}
};
