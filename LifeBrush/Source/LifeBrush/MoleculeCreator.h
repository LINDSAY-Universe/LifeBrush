// Copyright 2019, Timothy Davison. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Simulation/FlexElements.h"
#include "ElementActor.h"
#include "ShipEditorSimulation/Graph.h"
#include "GameFramework/Actor.h"
#include "MoleculeCreator.generated.h"

UCLASS(BlueprintType)
class LIFEBRUSH_API AMoleculeCreator : public AActor
{
	GENERATED_BODY()
	
public:	

	
	void attachNodes();
	// Sets default values for this actor's properties
	AMoleculeCreator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
