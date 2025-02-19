﻿// Copyright (c) 2021 Arunoda Susiripala. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OpenLandInstancingRules.h"

#include "GameFramework/Actor.h"
#include "OpenLandMeshPolygonMeshProxy.h"
#include "Core/OpenLandMeshComponent.h"
#include "Compute/Types/ComputeMaterial.h"

#include "OpenLandMeshActor.generated.h"

UENUM(BlueprintType)
enum EOpenLandMeshVisibility
{
	MV_SHOW_ALWAYS = 0 UMETA(DisplayName="Show Always"),
	MV_HIDE_ALWAYS = 1 UMETA(DisplayName="Hide Always"),
	MV_HIDE_IN_GAME = 2 UMETA(DisplayName="Hide in Game"),
	MV_HIDE_IN_EDITOR = 3 UMETA(DisplayName="Hide in Editor"),
};

struct FLODInfo
{
	FOpenLandPolygonMeshBuildResultPtr MeshBuildResult = nullptr;
	int32 MeshSectionIndex = 0;
	int32 LODIndex = 0;
	bool bIsModifyReady = false;

	bool MakeModifyReady()
	{
		if (bIsModifyReady)
		{
			return false;
		}

		if (MeshBuildResult->CacheKey.IsEmpty())
		{
			return false;
		}

		bIsModifyReady = true;
		MeshBuildResult = MeshBuildResult->ShallowClone();
		if (MeshBuildResult->Target)
		{
			MeshBuildResult->Target = MeshBuildResult->Target->Clone();
		}

		return true;
	}
};

struct FSwitchLODsStatus
{
	bool bNeedLODVisibilityChange = false;
	bool bAsyncBuildStarted = false;
};

typedef TSharedPtr<FLODInfo> FLODInfoPtr;

UCLASS()
class OPENLANDMESH_API AOpenLandMeshActor : public AActor
{
	GENERATED_BODY()
	
	bool bMeshGenerated = false;
	bool bNeedToAsyncModifyMesh = false;
	FOpenLandPolygonMeshModifyStatus ModifyStatus = {};

	UPROPERTY(NonPIEDuplicateTransient);
	FString ObjectId;
	
	TArray<FLODInfoPtr> LODList;
	FLODInfoPtr CurrentLOD = nullptr;
	bool bNeedLODVisibilityChange = false;
	int32 AsyncBuildingLODIndex = -1;

	void RunAsyncModifyMeshProcess(float LastFrameTime);
	void RunSyncModifyMeshProcess();
	FSwitchLODsStatus SwitchLODs();
	void EnsureLODVisibility();
	FString MakeCacheKey(int32 CurrentSubdivisions) const;
	void MakeModifyReady();
	void FinishBuildMeshAsync();
	bool CanRenderMesh() const;

public:
	AOpenLandMeshActor();
	~AOpenLandMeshActor();
	FString GetObjectId() const { return ObjectId; }

protected:
	UPROPERTY(Transient)
	UOpenLandMeshPolygonMeshProxy* PolygonMesh;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="OpenLandMesh")
	UOpenLandMeshPolygonMeshProxy* GetPolygonMesh();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="OpenLandMesh")
	FVertexModifierResult OnModifyVertex(FVertexModifierPayload Payload);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="OpenLandMesh")
    void OnAfterAnimations();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="OpenLandMesh")
	FString GetCacheKey() const;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	void SetMaterial(UMaterialInterface* Material);
	virtual bool ShouldTickIfViewportsOnly() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	void BuildMeshAsync(int32 LODIndex);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Rendering", Transient)
	UOpenLandMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	int32 SubDivisions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	float SmoothNormalAngle = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bRunCpuVertexModifiers = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	FComputeMaterial GpuVertexModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bRunGpuVertexModifiers = false;;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	UMaterialInterface* Material;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bAnimate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bDisableGPUVertexModifiersOnAnimate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bUseAsyncBuildMeshOnGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bEnableCollision = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bUseAsyncCollisionCooking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	bool bUseAsyncAnimations = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	int32 DesiredFrameRateOnModify = 60;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=OpenLandMesh)
	TEnumAsByte<EOpenLandMeshVisibility> MeshVisibility = MV_SHOW_ALWAYS;
	
	UPROPERTY(VisibleAnywhere, Category="OpenLandMesh LODs")
	int32 CurrentLODIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh LODs")
	int32 MaximumLODCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh LODs")
	int32 LODStepUnits = 3000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh LODs")
	float LODStepPower = 1.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh LODs")
	int32 LODIndexForCollisions = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh Instancing")
	TArray<FOpenLandInstancingRules> InstancingGroups;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="OpenLandMesh Instancing")
	bool bRunInstancingAfterBuildMesh = true;
	
	UFUNCTION(CallInEditor, BlueprintCallable, Category=OpenLandMesh)
	void BuildMesh();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLandMesh LODs")
	void RebuildLODs();

	UFUNCTION(CallInEditor, BlueprintCallable, Category=OpenLandMesh)
	void ModifyMesh();

	UFUNCTION(CallInEditor, BlueprintCallable, Category=OpenLandMesh)
	void ResetCache();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLandMesh Instancing")
	void ApplyInstances();

	UFUNCTION(CallInEditor, BlueprintCallable, Category="OpenLandMesh Instancing")
	void RemoveInstances();

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	void ModifyMeshAsync();

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
	void SetGPUScalarParameter(FName Name, float Value);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    float GetGPUScalarParameter(FName Name);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    void SetGPUVectorParameter(FName Name, FVector Value);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    FVector GetGPUVectorParameter(FName Name);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    void SetGPUTextureParameter(FName Name, UTexture2D* Value);

	UFUNCTION(BlueprintCallable, Category=OpenLandMesh)
    UTexture2D* GetGPUTextureParameter(FName Name);
};
