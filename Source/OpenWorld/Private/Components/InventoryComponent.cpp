// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/InventoryComponent.h"
#include "Characters/PlayerCharacter.h"
#include "Weapons/MeleeWeapon.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// ========================= Lifecycles ==================== //

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = Cast<APlayerCharacter>(GetOwner());
}

// ========================= Weaponary ==================== //

void UInventoryComponent::AddWeapon(AMeleeWeapon* NewWeapon)
{
	int8 Slot = FindEmptySlot();

	if (Slot == -1) return;

	Weapons[Slot] = NewWeapon;

	// Determine the socket
	FName SocketName = *FString::Printf(TEXT("Back%d Socket"), Slot);
	Weapons[Slot]->Pickup(PlayerCharacter.Get(), SocketName);
}

void UInventoryComponent::ChangeWeapon(int8 Slot)
{
	LastWeapon    = CurrentWeapon;
	CurrentWeapon = Slot;

	// When the player is pressing the same slot (for example, used weapon is slot 1 and then pressed 1 on keyboard)
	// Then what happened next is deattaching the weapon
	bool bShouldEquip = LastWeapon != CurrentWeapon ? true : !PlayerCharacter->IsEquippingWeapon();

	if (Weapons[LastWeapon].IsValid()) 
		GetWorld()->GetTimerManager().SetTimer(UnequipingTimerHandle, this, &ThisClass::UnequipWeapon, .6f);

	PlayerCharacter->SetCarriedWeapon(bShouldEquip ? Weapons[CurrentWeapon] : nullptr, Weapons[CurrentWeapon].IsValid());
}

void UInventoryComponent::UnequipWeapon()
{
	if (LastWeapon != CurrentWeapon || !PlayerCharacter->IsEquippingWeapon())
		Weapons[LastWeapon]->EquipTo(false);
}

int8 UInventoryComponent::FindEmptySlot()
{
	for (int8 I = 0; I < WeaponMaxSlot; ++I)
		if (Weapons[I] == nullptr) return I;

	return -1;
}
