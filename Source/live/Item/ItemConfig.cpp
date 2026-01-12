// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/ItemConfig.h"

FItemData FItemConfigEntry::ToItemData() const
{
	FItemData ItemData;
	ItemData.ItemID = ItemID;
	ItemData.ItemName = FName(ItemName.ToString());
	ItemData.Description = Description;
	ItemData.ItemType = ItemType;
	ItemData.EquipmentSlot = EquipmentSlot;
	ItemData.MaxStackSize = MaxStackSize;
	ItemData.GoldCost = GoldCost;
	ItemData.StatModifiers = StatModifiers;
	ItemData.GameplayEffectClass = GameplayEffectClass;
	ItemData.GrantedAbility = GrantedAbility;
	ItemData.Icon = Icon;
	ItemData.ItemTags = ItemTags;
	ItemData.bCanBeSold = bCanBeSold;
	ItemData.bCanBeDropped = bCanBeDropped;
	return ItemData;
}
