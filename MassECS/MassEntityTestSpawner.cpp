// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassECS/MassEntityTestSpawner.h"
#include "MassECS/MassEntityTestFragments.h"
#include "MassCommonFragments.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityManager.h"
#include "Components/InstancedStaticMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MassEntityTestSpawner)

AMassEntityTestSpawner::AMassEntityTestSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* const RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootSceneComponent);

	MeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("UnitMesh"));
	MeshComponent->SetupAttachment(RootSceneComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCastShadow(false);
}

void AMassEntityTestSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (InitialCount > 0 && EntityConfig)
	{
		SpawnUnits(InitialCount);
	}
}

void AMassEntityTestSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyAllUnits();
	Super::EndPlay(EndPlayReason);
}

void AMassEntityTestSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateVisualization();
}

void AMassEntityTestSpawner::SetupMeshFromEntity(const FMassEntityHandle& Entity)
{
	if (bMeshInitialized)
	{
		return;
	}

	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	UMassSpawnerSubsystem* SpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
	if (!SpawnerSubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = SpawnerSubsystem->GetEntityManagerChecked();
	const FMassEntityTestVisualParams* VisualParams = EntityManager.GetConstSharedFragmentDataPtr<FMassEntityTestVisualParams>(Entity);
	if (!VisualParams || !VisualParams->Mesh)
	{
		return;
	}

	MeshComponent->SetStaticMesh(VisualParams->Mesh);
	MeshComponent->SetWorldScale3D(FVector(VisualParams->Scale));

	if (VisualParams->Material)
	{
		MeshComponent->SetMaterial(0, VisualParams->Material);
	}

	bMeshInitialized = true;
}

void AMassEntityTestSpawner::AddInstanceForEntity(const FMassEntityHandle& Entity, const FTransform& Transform)
{
	const int32 InstanceIndex = MeshComponent->AddInstance(Transform, true);
	EntityToInstanceMap.Add(Entity, InstanceIndex);
	InstanceToEntityArray.Add(Entity);
}

void AMassEntityTestSpawner::RemoveInstanceForEntity(const FMassEntityHandle& Entity)
{
	const int32* const FoundIndex = EntityToInstanceMap.Find(Entity);
	if (!FoundIndex)
	{
		return;
	}

	const int32 RemovedIndex = *FoundIndex;
	const int32 LastIndex = InstanceToEntityArray.Num() - 1;

	MeshComponent->RemoveInstance(RemovedIndex);

	if (RemovedIndex < LastIndex)
	{
		const FMassEntityHandle SwappedEntity = InstanceToEntityArray[LastIndex];
		InstanceToEntityArray[RemovedIndex] = SwappedEntity;
		EntityToInstanceMap[SwappedEntity] = RemovedIndex;
	}

	InstanceToEntityArray.Pop();
	EntityToInstanceMap.Remove(Entity);
}

void AMassEntityTestSpawner::UpdateVisualization()
{
	if (!MeshComponent || !bMeshInitialized)
	{
		return;
	}

	UWorld* const World = GetWorld();
	if (!World)
	{
		return;
	}

	UMassSpawnerSubsystem* const SpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
	if (!SpawnerSubsystem)
	{
		return;
	}

	FMassEntityManager& EntityManager = SpawnerSubsystem->GetEntityManagerChecked();

	TArray<FMassEntityHandle> DeadEntities;
	for (const auto& Pair : EntityToInstanceMap)
	{
		if (!EntityManager.IsEntityValid(Pair.Key))
		{
			DeadEntities.Add(Pair.Key);
		}
	}

	for (const FMassEntityHandle& DeadEntity : DeadEntities)
	{
		RemoveInstanceForEntity(DeadEntity);
	}

	const int32 InstanceCount = InstanceToEntityArray.Num();
	if (InstanceCount == 0)
	{
		return;
	}

	TArray<FTransform> InstanceTransforms;
	InstanceTransforms.SetNum(InstanceCount);

	for (int32 Index = 0; Index < InstanceCount; ++Index)
	{
		const FMassEntityHandle& Entity = InstanceToEntityArray[Index];
		const FTransformFragment* const TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		if (TransformFragment)
		{
			InstanceTransforms[Index] = TransformFragment->GetTransform();
		}
	}

	MeshComponent->BatchUpdateInstancesTransforms(0, InstanceTransforms, true, true, true);
}

void AMassEntityTestSpawner::SpawnUnits(const int32 Count)
{
	UWorld* const World = GetWorld();
	if (!World || !EntityConfig)
	{
		return;
	}

	UMassSpawnerSubsystem* const SpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
	if (!SpawnerSubsystem)
	{
		return;
	}

	const FMassEntityTemplate& Template = EntityConfig->GetOrCreateEntityTemplate(*World);
	TArray<FMassEntityHandle> NewEntities;
	const TSharedPtr<FMassEntityManager::FEntityCreationContext> CreationContext = SpawnerSubsystem->SpawnEntities(Template, Count, NewEntities);
	FMassEntityManager& EntityManager = SpawnerSubsystem->GetEntityManagerChecked();

	for (int32 Index = 0; Index < NewEntities.Num(); ++Index)
	{
		const FMassEntityHandle& Entity = NewEntities[Index];
		const FVector SpawnPosition = CalcSpawnPosition(Index, Count);

		FTransformFragment* const TransformFragment = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
		if (TransformFragment)
		{
			TransformFragment->SetTransform(FTransform(SpawnPosition));
		}

		FMassEntityTestTeamFragment* const TeamFragment = EntityManager.GetFragmentDataPtr<FMassEntityTestTeamFragment>(Entity);
		if (TeamFragment)
		{
			TeamFragment->TeamId = TeamId;
		}
	}

	if (!bMeshInitialized && !NewEntities.IsEmpty())
	{
		SetupMeshFromEntity(NewEntities[0]);
	}

	InstanceToEntityArray.Reserve(NewEntities.Num());

	for (int32 Index = 0; Index < NewEntities.Num(); ++Index)
	{
		AddInstanceForEntity(NewEntities[Index], FTransform(CalcSpawnPosition(Index, Count)));
	}

	SpawnedEntities.Append(NewEntities);
	UE_LOG(LogTemp, Log, TEXT("[MassEntityTestSpawner] Spawned %d entities (Team %d)"), Count, TeamId);
}

void AMassEntityTestSpawner::DestroyAllUnits()
{
	UWorld* const World = GetWorld();
	if (!World || SpawnedEntities.IsEmpty())
	{
		return;
	}

	UMassSpawnerSubsystem* const SpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
	if (SpawnerSubsystem)
	{
		SpawnerSubsystem->DestroyEntities(SpawnedEntities);
	}

	MeshComponent->ClearInstances();
	EntityToInstanceMap.Reset();
	InstanceToEntityArray.Reset();
	SpawnedEntities.Reset();
}

int32 AMassEntityTestSpawner::GetAliveCount() const
{
	return InstanceToEntityArray.Num();
}

FVector AMassEntityTestSpawner::CalcSpawnPosition(const int32 Index, const int32 Total) const
{
	const float GoldenAngle = PI * (3.f - FMath::Sqrt(5.f));
	const float Theta = GoldenAngle * Index;
	const float Radius = SpawnRadius * FMath::Sqrt(static_cast<float>(Index) / FMath::Max(Total, 1));
	return GetActorLocation() + FVector(Radius * FMath::Cos(Theta), Radius * FMath::Sin(Theta), 0.f);
}
