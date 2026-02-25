// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassEntityTraitBase.h"
#include "MassECS/MassEntityTestFragments.h"
#include "MassEntityTestCombatTrait.generated.h"

UCLASS(meta = (DisplayName = "MassEntityTest Combat"))
class LYRAGAME_API UMassEntityTestCombatTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Combat|Health")
	FMassEntityTestHealthFragment Health;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	FMassEntityTestCombatFragment Combat;

	UPROPERTY(EditAnywhere, Category = "Combat|Team")
	FMassEntityTestTeamFragment Team;

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	FMassEntityTestMoveFragment Movement;

	UPROPERTY(EditAnywhere, Category = "Combat|Visual")
	FMassEntityTestVisualParams Visual;

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
