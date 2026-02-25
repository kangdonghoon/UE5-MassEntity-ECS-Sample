// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassECS/MassEntityTestCombatProcessor.h"
#include "MassECS/MassEntityTestFragments.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MassEntityTestCombatProcessor)

UMassEntityTestCombatProcessor::UMassEntityTestCombatProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassEntityTestMovementProcessor"));
}

void UMassEntityTestCombatProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	AttackerQuery.Initialize(EntityManager);
	AttackerQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	AttackerQuery.AddRequirement<FMassEntityTestCombatFragment>(EMassFragmentAccess::ReadWrite);
	AttackerQuery.AddRequirement<FMassEntityTestTargetFragment>(EMassFragmentAccess::ReadWrite);
	AttackerQuery.AddRequirement<FMassEntityTestTeamFragment>(EMassFragmentAccess::ReadOnly);
	AttackerQuery.AddTagRequirement<FMassEntityTestDeadTag>(EMassFragmentPresence::None);
	AttackerQuery.RegisterWithProcessor(*this);

	CandidateQuery.Initialize(EntityManager);
	CandidateQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	CandidateQuery.AddRequirement<FMassEntityTestTeamFragment>(EMassFragmentAccess::ReadOnly);
	CandidateQuery.AddRequirement<FMassEntityTestHealthFragment>(EMassFragmentAccess::ReadOnly);
	CandidateQuery.AddTagRequirement<FMassEntityTestDeadTag>(EMassFragmentPresence::None);
	CandidateQuery.RegisterWithProcessor(*this);
}

void UMassEntityTestCombatProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	const float DeltaTime = Context.GetDeltaTimeSeconds();
	TArray<FCandidateInfo> Candidates;

	CandidateQuery.ForEachEntityChunk(Context, [&Candidates](FMassExecutionContext& ChunkContext)
	{
		const int32 EntityCount = ChunkContext.GetNumEntities();
		const auto TransformList = ChunkContext.GetFragmentView<FTransformFragment>();
		const auto TeamList = ChunkContext.GetFragmentView<FMassEntityTestTeamFragment>();

		for (int32 Index = 0; Index < EntityCount; ++Index)
		{
			Candidates.Add({
				ChunkContext.GetEntity(Index),
				TransformList[Index].GetTransform().GetLocation(),
				TeamList[Index].TeamId
			});
		}
	});

	TArray<FMassEntityHandle> NewlyDeadEntities;

	AttackerQuery.ForEachEntityChunk(Context, [&EntityManager, &Candidates, &NewlyDeadEntities, DeltaTime](FMassExecutionContext& ChunkContext)
	{
		const int32 EntityCount = ChunkContext.GetNumEntities();
		const auto TransformList = ChunkContext.GetFragmentView<FTransformFragment>();
		auto CombatList = ChunkContext.GetMutableFragmentView<FMassEntityTestCombatFragment>();
		auto TargetList = ChunkContext.GetMutableFragmentView<FMassEntityTestTargetFragment>();
		const auto TeamList = ChunkContext.GetFragmentView<FMassEntityTestTeamFragment>();

		for (int32 Index = 0; Index < EntityCount; ++Index)
		{
			FMassEntityTestCombatFragment& CombatFragment = CombatList[Index];
			FMassEntityTestTargetFragment& TargetFragment = TargetList[Index];
			CombatFragment.TimeSinceLastAttack += DeltaTime;

			if (TargetFragment.HasTarget() && !EntityManager.IsEntityValid(TargetFragment.TargetEntity))
			{
				TargetFragment.ClearTarget();
			}

			if (!TargetFragment.HasTarget())
			{
				const FVector MyPosition = TransformList[Index].GetTransform().GetLocation();
				const int32 MyTeamId = TeamList[Index].TeamId;
				const float AttackRangeSquared = FMath::Square(CombatFragment.AttackRange);
				float BestDistanceSquared = MAX_FLT;

				for (const FCandidateInfo& Candidate : Candidates)
				{
					if (Candidate.TeamId == MyTeamId)
					{
						continue;
					}

					const float DistanceSquared = FVector::DistSquared(MyPosition, Candidate.Location);
					if (DistanceSquared <= AttackRangeSquared && DistanceSquared < BestDistanceSquared)
					{
						BestDistanceSquared = DistanceSquared;
						TargetFragment.TargetEntity = Candidate.Entity;
					}
				}
			}

			if (TargetFragment.HasTarget() && CombatFragment.CanAttack())
			{
				FMassEntityTestHealthFragment* const TargetHealth = EntityManager.GetFragmentDataPtr<FMassEntityTestHealthFragment>(TargetFragment.TargetEntity);
				if (TargetHealth && !TargetHealth->IsDead())
				{
					TargetHealth->CurrentHealth -= CombatFragment.AttackDamage;
					CombatFragment.ResetCooldown();

					if (TargetHealth->IsDead())
					{
						TargetHealth->CurrentHealth = 0.f;
						NewlyDeadEntities.Add(TargetFragment.TargetEntity);
						TargetFragment.ClearTarget();
					}
				}
				else
				{
					TargetFragment.ClearTarget();
				}
			}
		}
	});

	for (const FMassEntityHandle& DeadEntity : NewlyDeadEntities)
	{
		if (EntityManager.IsEntityValid(DeadEntity))
		{
			Context.Defer().AddTag<FMassEntityTestDeadTag>(DeadEntity);
		}
	}
}
