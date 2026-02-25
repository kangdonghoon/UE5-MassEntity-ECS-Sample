// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassEntityTestMovementProcessor.generated.h"

UCLASS()
class LYRAGAME_API UMassEntityTestMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassEntityTestMovementProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery MoverQuery;
	FMassEntityQuery EnemyQuery;

	struct FEnemyInfo
	{
		FMassEntityHandle Entity;
		FVector Location;
		int32 TeamId;
	};
};
