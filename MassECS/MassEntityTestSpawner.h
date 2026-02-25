// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTypes.h"
#include "MassEntityTestSpawner.generated.h"

class UMassSpawnerSubsystem;
class UInstancedStaticMeshComponent;

UCLASS(Blueprintable)
class LYRAGAME_API AMassEntityTestSpawner : public AActor
{
	GENERATED_BODY()

public:
	AMassEntityTestSpawner();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "MassECS")
	void SpawnUnits(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "MassECS")
	void DestroyAllUnits();

	UFUNCTION(BlueprintPure, Category = "MassECS")
	int32 GetAliveCount() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "MassECS|Config")
	TObjectPtr<UMassEntityConfigAsset> EntityConfig;

	UPROPERTY(EditAnywhere, Category = "MassECS|Config", meta = (ClampMin = "0", ClampMax = "10000"))
	int32 InitialCount = 50;

	UPROPERTY(EditAnywhere, Category = "MassECS|Config")
	int32 TeamId = 0;

	UPROPERTY(EditAnywhere, Category = "MassECS|Config", meta = (ClampMin = "0"))
	float SpawnRadius = 1000.f;

private:
	FVector CalcSpawnPosition(int32 Index, int32 Total) const;
	void SetupMeshFromEntity(const FMassEntityHandle& Entity);
	void AddInstanceForEntity(const FMassEntityHandle& Entity, const FTransform& Transform);
	void RemoveInstanceForEntity(const FMassEntityHandle& Entity);
	void UpdateVisualization();

	UPROPERTY(VisibleAnywhere, Category = "MassECS|Visual")
	TObjectPtr<UInstancedStaticMeshComponent> MeshComponent;

	UPROPERTY(Transient)
	TArray<FMassEntityHandle> SpawnedEntities;

	TMap<FMassEntityHandle, int32> EntityToInstanceMap;
	TArray<FMassEntityHandle> InstanceToEntityArray;

	bool bMeshInitialized = false;
};
