#pragma once
#include "ShipEditorSimulation/ObjectSimulation.h"
#include "Simulation/FlexGraphSimulation_interface.h"
#include "ImmuneSysSimulation.generated.h"


UENUM()
enum EImmune_PathogenState {Free,MacrophageBound};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FImmune_Pathogen : public FGraphObject
{
	GENERATED_BODY()
	public:
	EImmune_PathogenState PathogenState = EImmune_PathogenState::Free;
	
};


UENUM()
enum EImmune_TCellState { SeekingMacrophage,Activated, BCellBound  };

//CD4
USTRUCT(BlueprintType)
struct LIFEBRUSH_API FImmune_HelperT : public  FGraphObject
{
	GENERATED_BODY()
	public:
	EImmune_TCellState TCellState = SeekingMacrophage;
	
	//ref to bcell this t cell is bound to
	FGraphNode* boundBCell = nullptr;
	FGraphNode* boundMacro = nullptr;

	UPROPERTY(EditAnywhere, Category="Immune")
	float bindingRadius = 10.0f;

	UPROPERTY(EditAnywhere,Category="Immune")
	float interactionRadius;

	UPROPERTY(EditAnywhere, Category="Random Walk")
	float baseVelocity = 10.0f;

	UPROPERTY(EditAnywhere, Category="Random Walk")
	float maxOffset = 5.0f;

};

UENUM()
enum EImmune_BCellState {Unbound,TCellBound };

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FImmune_NaiveB : public FGraphObject
{
	GENERATED_BODY()
	public:

	
	EImmune_BCellState BCellState = EImmune_BCellState::Unbound;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Immune")
	TArray<FTimStructBox> memCellTemplate;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Immune")
	TArray<FTimStructBox> plasmaCellTemplate;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Immune")
	float interactionRadius = 5.0f;

	UPROPERTY(EditAnywhere,Category="Immune")
	float bindingRadius = 1.0f;

	UPROPERTY(EditAnywhere,Category = "Random Walk")
	float baseVelocity = 10.0f;

	UPROPERTY(EditAnywhere,Category = "Random Walk")
	float maxOffset = 5.0f;

	

};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FImmune_PlasmaCells : public FImmune_NaiveB
{
	GENERATED_BODY()
	public:
	
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FImmune_MemoryCells : public FImmune_NaiveB
{
	GENERATED_BODY()
	public:
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FImmune_Antibody : public FGraphObject
{
	GENERATED_BODY()
	public:
	FImmune_PlasmaCells* parentPlasmaCell;
};

UENUM()
enum EImmune_MacrophageState {MacroFree,MacroPathogenBound  };

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FImmune_Macrophage : public FGraphObject
{
	GENERATED_BODY()
	public:

	EImmune_MacrophageState MacrophageState = EImmune_MacrophageState::MacroFree;
	
	UPROPERTY(EditAnywhere,Category="Immune")
	float interactionRadius;

	UPROPERTY(EditAnywhere,Category="Immune")
	float bindingRadius;


	UPROPERTY(EditAnywhere,Category="Random Walk")
	float baseVelocity = 10.0f;

	UPROPERTY(EditAnywhere,Category="Random Walk")
	float maxOffset = 5.0f;

	private:


	

};

UCLASS(BlueprintType)
class UImmuneSysSimulation : public UObjectSimulation, public IFlexGraphSimulation
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere,Category="Immune")
	TArray<FTimStructBox> cytokineTemplate;

	UPROPERTY(EditAnywhere,Category="Immune")
	int numCytokinesToSpawn;

	UPROPERTY(EditAnywhere,Category="Immune")
	TArray<FTimStructBox> plasmaCellTemplate;

	UPROPERTY(EditAnywhere,Category="Immune")
	TArray<FTimStructBox> memoryCellTemplate;

	UPROPERTY(EditAnywhere,Category="Immune")
	TArray<FTimStructBox> antibodyTemplate;

	

protected:
	virtual void attach() override;

	//T CELL FUNCTIONS
	void HelperTBehaviour(TypedComponentStorage<FImmune_HelperT>,TypedComponentStorage<FImmune_NaiveB>,
		TypedComponentStorage<FImmune_Macrophage>,
		NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,int);

	void TCellSeekingMacrophage(FImmune_HelperT tCell, TypedComponentStorage<FImmune_Macrophage> Macrophages,
		NvFlexVector<int>& neighbourIndices, NvFlexVector<int>& neighbourCounts,
		NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI, int maxParticles);
	
	void TCellActivated(FImmune_HelperT tCell, TypedComponentStorage<FImmune_NaiveB> BCells,
        NvFlexVector<int>& neighbourIndices, NvFlexVector<int>& neighbourCounts,
        NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI, int maxParticles);
	
	void TCellBCellBound(FImmune_HelperT);


	//B CELL FUNCTIONS
	void NaiveBCellBehavior(TypedComponentStorage<FImmune_NaiveB>,TypedComponentStorage<FImmune_HelperT>,
	NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,int);

	void BCellTCellBoundState(FImmune_NaiveB,TypedComponentStorage<FImmune_HelperT>,
		NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,int);

	void BCellUnBoundState(FImmune_NaiveB bCell, TypedComponentStorage<FImmune_HelperT>,
        NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,int);


	//MACROPHAGE FUNCTIONS
	void MacrophageBehavior(TypedComponentStorage<FImmune_Macrophage>,TypedComponentStorage<FImmune_Pathogen>,
    NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,int);

	void MacroFreeState(FImmune_Macrophage,TypedComponentStorage<FImmune_Pathogen>,
		NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,int);

	void MacroBoundState(FImmune_Macrophage,
		NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,NvFlexVector<int>&,int);

	
	
	void BCellEngulfPathogen(FGraphNode&,FGraphNode&);

	void BindMolecules(FGraphNode&, FGraphNode&);

	void UnBindMolecules(FGraphNode&,FGraphNode&);


	FGraphNode& SpawnMolecule(FVector,FQuat,TArray<FTimStructBox>);

public:
	virtual void tick(float deltaT) override;

	virtual void flexTick(float deltaT,
		NvFlexVector<int>& neighbourIndices,
		NvFlexVector<int>& neighbourCounts,
		NvFlexVector<int>& apiToInternal,
		NvFlexVector<int>& internalToAPI,
		int maxParticles) override;

	
	
};
