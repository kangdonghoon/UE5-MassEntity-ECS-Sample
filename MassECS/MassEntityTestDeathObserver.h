// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MassObserverProcessor.h"
#include "MassEntityQuery.h"
#include "MassEntityTestDeathObserver.generated.h"

UCLASS()
class LYRAGAME_API UMassEntityTestDeathObserver : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UMassEntityTestDeathObserver();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery DeadEntityQuery;
};
