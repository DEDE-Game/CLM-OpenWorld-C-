#pragma once

enum class ECharacterState : uint8
{
    ECS_NoAction,
    ECS_Action, // Can be attacking, Equipping, Kicking, etc
    ECS_Died
};
