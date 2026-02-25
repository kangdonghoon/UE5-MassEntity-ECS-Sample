// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassECS/MassEntityTestMovementProcessor.h"
#include "MassECS/MassEntityTestFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MassEntityTestMovementProcessor)

UMassEntityTestMovementProcessor::UMassEntityTestMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassEntityTestCombatProcessor"));
}

void UMassEntityTestMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EnemyQuery.Initialize(EntityManager);
	EnemyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EnemyQuery.AddRequirement<FMassEntityTestTeamFragment>(EMassFragmentAccess::ReadOnly);
	EnemyQuery.AddTagRequirement<FMassEntityTestDeadTag>(EMassFragmentPresence::None);
	EnemyQuery.RegisterWithProcessor(*this);

	MoverQuery.Initialize(EntityManager);
	MoverQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	MoverQuery.AddRequirement<FMassEntityTestMoveFragment>(EMassFragmentAccess::ReadOnly);
	MoverQuery.AddRequirement<FMassEntityTestTeamFragment>(EMassFragmentAccess::ReadOnly);
	MoverQuery.AddRequirement<FMassEntityTestCombatFragment>(EMassFragmentAccess::ReadOnly);
	MoverQuery.AddTagRequirement<FMassEntityTestDeadTag>(EMassFragmentPresence::None);
	MoverQuery.RegisterWithProcessor(*this);
}

void UMassEntityTestMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FEnemyInfo> AllUnits;

	EnemyQuery.ForEachEntityChunk(Context, [&AllUnits](FMassExecutionContext& ChunkContext)
	{
		const int32 EntityCount = ChunkContext.GetNumEntities();
		const auto TransformList = ChunkContext.GetFragmentView<FTransformFragment>();
		const auto TeamList = ChunkContext.GetFragmentView<FMassEntityTestTeamFragment>();

		for (int32 Index = 0; Index < EntityCount; ++Index)
		{
			AllUnits.Add({
				ChunkContext.GetEntity(Index),
				TransformList[Index].GetTransform().GetLocation(),
				TeamList[Index].TeamId
			});
		}
	});

	MoverQuery.ForEachEntityChunk(Context, [&AllUnits](FMassExecutionContext& ChunkContext)
	{
		const float DeltaTime = ChunkContext.GetDeltaTimeSeconds();
		const int32 EntityCount = ChunkContext.GetNumEntities();
		auto TransformList = ChunkContext.GetMutableFragmentView<FTransformFragment>();
		const auto MoveList = ChunkContext.GetFragmentView<FMassEntityTestMoveFragment>();
		const auto TeamList = ChunkContext.GetFragmentView<FMassEntityTestTeamFragment>();
		const auto CombatList = ChunkContext.GetFragmentView<FMassEntityTestCombatFragment>();

		for (int32 Index = 0; Index < EntityCount; ++Index)
		{
			FTransform& EntityTransform = TransformList[Index].GetMutableTransform();
			const FVector CurrentPosition = EntityTransform.GetLocation();
			const int32 MyTeamId = TeamList[Index].TeamId;
			const float AttackRangeSquared = FMath::Square(CombatList[Index].AttackRange);
			const float MoveSpeed = MoveList[Index].MoveSpeed;

			float BestDistanceSquared = MAX_FLT;
			FVector BestEnemyPosition = CurrentPosition;
			bool bFoundEnemy = false;

			for (const FEnemyInfo& Unit : AllUnits)
			{
				if (Unit.TeamId == MyTeamId)
				{
					continue;
				}

				const float DistanceSquared = FVector::DistSquared(CurrentPosition, Unit.Location);
				if (DistanceSquared < BestDistanceSquared)
				{
					BestDistanceSquared = DistanceSquared;
					BestEnemyPosition = Unit.Location;
					bFoundEnemy = true;
				}
			}

			if (!bFoundEnemy)
			{
				continue;
			}

			const FVector Direction = (BestEnemyPosition - CurrentPosition).GetSafeNormal2D();
			if (!Direction.IsNearlyZero())
			{
				EntityTransform.SetRotation(FQuat(Direction.Rotation() + FRotator(0.f, -90.f, 0.f)));
			}

			if (BestDistanceSquared > AttackRangeSquared)
			{
				const float MaxStep = MoveSpeed * DeltaTime;
				const FVector MoveDelta = Direction * FMath::Min(MaxStep, FMath::Sqrt(BestDistanceSquared));
				FVector NewPosition = CurrentPosition + MoveDelta;
				NewPosition.Z = CurrentPosition.Z;
				EntityTransform.SetLocation(NewPosition);
			}
		}
	});
}
