// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassEntityTestHealthProcessor.generated.h"

UCLASS()
class LYRAGAME_API UMassEntityTestHealthRegenProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UMassEntityTestHealthRegenProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery HealthQuery;
};
