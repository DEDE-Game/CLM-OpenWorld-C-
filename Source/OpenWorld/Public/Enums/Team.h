#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
    T_Neutral UMETA(DisplayName="Neutral"),
    T_Friend  UMETA(DisplayName="Friend"),
    T_Enemy   UMETA(DisplayName="Enemy")
};
