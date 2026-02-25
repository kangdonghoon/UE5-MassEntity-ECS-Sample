// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassEntityTypes.h"
#include "MassEntityHandle.h"
#include "Engine/StaticMesh.h"
#include "MassEntityTestFragments.generated.h"

// ---------------------------------------------------------------------------
// Custom Fragments
// ---------------------------------------------------------------------------

USTRUCT()
struct LYRAGAME_API FMassEntityTestHealthFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Health")
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = "0"))
	float RegenPerSecond = 2.f;

	bool IsDead() const { return CurrentHealth <= 0.f; }
};

USTRUCT()
struct LYRAGAME_API FMassEntityTestCombatFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (ClampMin = "0"))
	float AttackCooldown = 1.f;

	float TimeSinceLastAttack = 0.f;

	bool CanAttack() const { return TimeSinceLastAttack >= AttackCooldown; }
	void ResetCooldown() { TimeSinceLastAttack = 0.f; }
};

USTRUCT()
struct LYRAGAME_API FMassEntityTestTargetFragment : public FMassFragment
{
	GENERATED_BODY()

	FMassEntityHandle TargetEntity;

	bool HasTarget() const { return TargetEntity.IsValid(); }
	void ClearTarget() { TargetEntity = FMassEntityHandle(); }
};

USTRUCT()
struct LYRAGAME_API FMassEntityTestTeamFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Team")
	int32 TeamId = 0;
};

USTRUCT()
struct LYRAGAME_API FMassEntityTestMoveFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0"))
	float MoveSpeed = 300.f;
};

// ---------------------------------------------------------------------------
// Shared Fragment — 같은 아키타입의 모든 엔티티가 공유하는 데이터
// ---------------------------------------------------------------------------

USTRUCT()
struct LYRAGAME_API FMassEntityTestVisualParams : public FMassConstSharedFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Visual")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TObjectPtr<UMaterialInterface> Material = nullptr;

	UPROPERTY(EditAnywhere, Category = "Visual", meta = (ClampMin = "0.01"))
	float Scale = 0.3f;
};

// ---------------------------------------------------------------------------
// Tags
// ---------------------------------------------------------------------------

USTRUCT()
struct LYRAGAME_API FMassEntityTestDeadTag : public FMassTag
{
	GENERATED_BODY()
};

// ---------------------------------------------------------------------------
// Signal Names
// ---------------------------------------------------------------------------

namespace MassEntityTestSignals
{
	inline const FName OnDeath = TEXT("MassEntityTest.OnDeath");
}
