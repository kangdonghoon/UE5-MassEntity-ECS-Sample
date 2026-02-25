// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassEntityTestCombatProcessor.generated.h"

UCLASS()
class LYRAGAME_API UMassEntityTestCombatProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassEntityTestCombatProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery AttackerQuery;
	FMassEntityQuery CandidateQuery;

	struct FCandidateInfo
	{
		FMassEntityHandle Entity;
		FVector Location;
		int32 TeamId;
	};
};
