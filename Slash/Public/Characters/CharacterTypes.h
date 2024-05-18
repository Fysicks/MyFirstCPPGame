#pragma once
UENUM(BlueprintType)
enum class ECharacterState : uint8 {
	// Convention is to prepend the capital letters of enum name to all enums 
	ECS_Unequipped UMETA(DisplayName = "Unequipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One Handed Weapon"),
	ECS_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two Handed Weapon")
};

UENUM(BlueprintType)
enum class EActionState : uint8 {
	// Convention is to prepend the capital letters of enum name to all enums 
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_Equipping UMETA(DisplayName = "Equipping_Weapon")
};

UENUM(BlueprintType)
enum class EDeathPose : uint8 {
	// Convention is to prepend the capital letters of enum name to all enums 
	EDP_Alive UMETA(DisplayName = "Alive"),
	EDP_Death1 UMETA(Display_name = "Death1"),
	EDP_Death2 UMETA(Display_name = "Death2"),
	EDP_Death3 UMETA(Display_name = "Death3"),
	EDP_Death4 UMETA(Display_name = "Death4"),
	EDP_Death5 UMETA(Display_name = "Death5"),
	EDP_Death6 UMETA(Display_name = "Death6")
};