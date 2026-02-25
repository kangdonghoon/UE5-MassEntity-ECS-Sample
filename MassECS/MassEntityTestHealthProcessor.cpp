// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassECS/MassEntityTestHealthProcessor.h"
#include "MassECS/MassEntityTestFragments.h"
#include "MassExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MassEntityTestHealthProcessor)

UMassEntityTestHealthRegenProcessor::UMassEntityTestHealthRegenProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassEntityTestCombatProcessor"));
}

void UMassEntityTestHealthRegenProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	HealthQuery.Initialize(EntityManager);
	HealthQuery.AddRequirement<FMassEntityTestHealthFragment>(EMassFragmentAccess::ReadWrite);
	HealthQuery.AddTagRequirement<FMassEntityTestDeadTag>(EMassFragmentPresence::None);
	HealthQuery.RegisterWithProcessor(*this);
}

void UMassEntityTestHealthRegenProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	HealthQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& ChunkContext)
	{
		const float DeltaTime = ChunkContext.GetDeltaTimeSeconds();
		const int32 EntityCount = ChunkContext.GetNumEntities();
		auto HealthList = ChunkContext.GetMutableFragmentView<FMassEntityTestHealthFragment>();

		for (int32 Index = 0; Index < EntityCount; ++Index)
		{
			FMassEntityTestHealthFragment& HealthFragment = HealthList[Index];
			if (HealthFragment.RegenPerSecond <= 0.f || HealthFragment.CurrentHealth >= HealthFragment.MaxHealth)
			{
				continue;
			}
			HealthFragment.CurrentHealth = FMath::Min(
				HealthFragment.CurrentHealth + HealthFragment.RegenPerSecond * DeltaTime,
				HealthFragment.MaxHealth);
		}
	});
}
