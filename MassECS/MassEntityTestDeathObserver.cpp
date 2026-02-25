// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassECS/MassEntityTestDeathObserver.h"
#include "MassECS/MassEntityTestFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MassEntityTestDeathObserver)

UMassEntityTestDeathObserver::UMassEntityTestDeathObserver()
{
	ObservedType = FMassEntityTestDeadTag::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Add;
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	bRequiresGameThreadExecution = true;
}

void UMassEntityTestDeathObserver::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	DeadEntityQuery.Initialize(EntityManager);
	DeadEntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	DeadEntityQuery.AddRequirement<FMassEntityTestTeamFragment>(EMassFragmentAccess::ReadOnly);
	DeadEntityQuery.AddTagRequirement<FMassEntityTestDeadTag>(EMassFragmentPresence::All);
	DeadEntityQuery.RegisterWithProcessor(*this);
}

void UMassEntityTestDeathObserver::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> EntitiesToDestroy;

	DeadEntityQuery.ForEachEntityChunk(Context, [&EntitiesToDestroy](FMassExecutionContext& ChunkContext)
	{
		const int32 EntityCount = ChunkContext.GetNumEntities();
		auto TransformList = ChunkContext.GetMutableFragmentView<FTransformFragment>();
		const auto TeamList = ChunkContext.GetFragmentView<FMassEntityTestTeamFragment>();

		for (int32 Index = 0; Index < EntityCount; ++Index)
		{
			UE_LOG(LogTemp, Log, TEXT("[ECS] Team %d unit destroyed at %s"),
				TeamList[Index].TeamId,
				*TransformList[Index].GetTransform().GetLocation().ToString());

			TransformList[Index].GetMutableTransform().SetScale3D(FVector::ZeroVector);
			EntitiesToDestroy.Add(ChunkContext.GetEntity(Index));
		}
	});

	for (const FMassEntityHandle& DeadEntity : EntitiesToDestroy)
	{
		if (EntityManager.IsEntityValid(DeadEntity))
		{
			Context.Defer().DestroyEntity(DeadEntity);
		}
	}
}
