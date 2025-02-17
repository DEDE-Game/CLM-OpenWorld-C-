// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enums/CharacterState.h"
#include "Enums/Team.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "OWCharacter.generated.h"

class AMeleeWeapon;
class USphereComponent;
class UNiagaraSystem;

UCLASS(Abstract)
class OPENWORLD_API AOWCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	AOWCharacter();

	// ***===== Lifecycles ==========*** //

	virtual void Tick(float DeltaTime) override;

	// ***===== Attributes ==========*** //
	
	void Stunned();

	// ***===== Combat ==========*** //

	//~ Begin IHitInterface
	virtual const bool IsEnemy(AOWCharacter* Other) const override;
	virtual const bool IsBlocking() const override
	{
		return bSucceedBlocking;
	}
	virtual void OnWeaponHit(AOWCharacter* DamagingCharacter, const FVector& ImpactPoint, const float GivenDamage, bool bBlockable) override;
	//~ End IHitInterface

	/** Used to deactivate any action such as takedown stealth */
	virtual void DeactivateAction() {}

protected:
	// ***===== Lifecycles ==========*** //

	virtual void BeginPlay() override;

	// ***===== Components ==========*** //

	UPROPERTY(VisibleAnywhere) 
	TObjectPtr<USphereComponent> KickHitbox;

	// ***===== Locomotions ==========*** //

	UPROPERTY(EditAnywhere, Category=Locomotions)
	float SprintSpeed = 800.f;

	UPROPERTY(EditAnywhere, Category=Locomotions)
	float RunSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category=Locomotions)
	float WalkSpeed = 200.f;

	FORCEINLINE void ToggleWalk(bool bToggled);
	FORCEINLINE void ToggleSprint(bool bToggled);
	FORCEINLINE void ToggleCrouch(bool bToggled);

	UFUNCTION(BlueprintCallable)
	void MoveForward();

	/** Re-enabling root motion after certain time */
	FTimerHandle RootMotionTimerHandler;

	// ***===== Attributes ==========*** //

	ECharacterState CharacterState = ECharacterState::ECS_NoAction;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ResetState();

	// *** Health Related *** //

	UPROPERTY(EditAnywhere, Category=Attributes)
	float MaxHealth = 100.f;

	float Health = MaxHealth;

	FORCEINLINE void SetHealth(float Offset)
	{
		Health = FMath::Clamp(Health + Offset, 0.f, MaxHealth);

		if (Health == 0.f) Die();
	}

	virtual void Die();
	
	// *** Stunned *** //

	FTimerHandle StunTimerHandle;
	
	FORCEINLINE void FinishedStunned();

	// ***===== Combat ==========*** //

	UPROPERTY(EditDefaultsOnly, Category=Combat)
	ETeam Team = ETeam::T_Neutral;

	UPROPERTY()
	TWeakObjectPtr<AMeleeWeapon> CarriedWeapon;

	UPROPERTY()
	TWeakObjectPtr<AOWCharacter> TargetCombat;

	bool bEquipWeapon = false;
	bool bSucceedBlocking = false;

	// *** Combo *** //
	int8 AttackCount  = 0;

	/** Combo timer, when its over so do the combo */
	FTimerHandle ComboOverHandler;

	UPROPERTY(EditAnywhere, Category=Combat)
	float ComboOverTimer = 2.f;

	virtual void AttackCombo();
	FORCEINLINE void ComboOver();

	// *** Weapon & Blocking *** //

	/** Check are the character allowed to swap weapon
	 * this can be dissalowed if the new equipped weapon is not valid
	 */
	bool bAllowSwapWeapon = true;

	/** Blocking opponent's attack */
	void ToggleBlock(bool bToggled);

	UFUNCTION(BlueprintCallable)
	virtual void EnableWeapon(bool bEnabled);

	/** Changing which weapon to equip by attaching to the character's back or hand */
	UFUNCTION(BlueprintCallable)
	virtual void AttachWeapon();

	/** About to change the weapon */
	void SwapWeapon();

	// *** Attacking/Engaging *** //

	/** If the oponent is farer than CombatRadius, it will not lock on that opponent anymore */
	UPROPERTY(EditAnywhere, Category=Combat)
	float CombatRadius = 1500.f;

	/**
	 * So it will always facing the target
	 * 
	 * @param Target If the target is nullptr, then the character wont lock on anything (Clear Lock On)
	 */
	FORCEINLINE virtual void SetLockOn(AOWCharacter* Target);

	/** Find nearest enemy then lock to him */
	void LockNearest();
	void LockOn(float DeltaTime);
	void HitReaction(const FVector& ImpactPoint, bool bBlockable);

	virtual void Attack();

	/** Called when the target is out of combat radius */
	virtual void OnLostInterest() {};

	// *** CHARGING ATTACK *** //

	UPROPERTY(EditAnywhere, Category=Combat)
	float DamageMultiplierRate = .8f;

	float DamageMultiplier = 1.f;

	/** Once the time is passed, will use charge attack instead */
	FTimerHandle ChargeTimerHandle;
	float ChargeAfter = .2f;
	float AutoChargeTimer = 1.f;

	bool bCharging = false;

	virtual void StartChargeAttack();
	void OnChargeAttack();
	void ChargeAttack();

	// *** Kicking *** //

	/** Kicking make enemy's block is toggled off */
	void StartKick();

	UFUNCTION()
	void OnKick(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// ***===== Animations ==========*** //

	UPROPERTY(EditDefaultsOnly, Category=Animations)
	TMap<FName, TSoftObjectPtr<UAnimMontage>> Montages;

	// ***===== Audio ==========*** //

	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TMap<FName, TSoftObjectPtr<USoundBase>> FootstepSounds;

	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TSoftObjectPtr<USoundBase> HitfleshSound;

	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TSoftObjectPtr<USoundBase> BlockingSound;

	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TSoftObjectPtr<USoundBase> KickingSound;

	UFUNCTION(BlueprintCallable)
	void PlayFootstepSound();

	// ***===== VFX ==========*** //

	UPROPERTY(EditDefaultsOnly, Category=VFX)
	TSoftObjectPtr<UNiagaraSystem> BloodSplash;

private:
	void DefaultInitializer();

public:
	// ***===== Accessors ==========*** //

	FORCEINLINE AOWCharacter* GetTargetCombat()
	{
		return TargetCombat.Get();
	}
	FORCEINLINE const bool IsReady() const
	{
		return CharacterState == ECharacterState::ECS_NoAction;
	}
	FORCEINLINE const bool IsEquippingWeapon() const
	{
		return bEquipWeapon;
	}
	FORCEINLINE const bool IsDead() const
	{
		return CharacterState == ECharacterState::ECS_Died;
	}
	FORCEINLINE const bool IsOnMontage(const FName& MontageName = TEXT("Any"))
	{
		UAnimMontage* Montage = GetCurrentMontage();
		if (MontageName == TEXT("Any")) return Montage != nullptr;

		if (!Montages.Contains(MontageName)) return false;

		return Montage ? Montage == Montages[MontageName].Get() : false;
	}
	FORCEINLINE ETeam GetTeam() const 
	{
		return Team;
	}
};
