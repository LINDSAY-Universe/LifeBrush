// Copyright (c) 2018 Timothy Davison. All rights reserved.

#pragma once

#include "Components/ActorComponent.h"

#include "Flex/include/NvFlex.h"
#include "Flex/include/NvFlexExt.h"
#include "Flex/include/NvFlexDevice.h"

#include "Simulation/FlexGraphSimulation_interface.h"

#include "ShipEditorSimulation/Graph.h"
#include "ShipEditorSimulation/ObjectSimulation.h"
#include "ShipEditorSimulation/GraphSnapshot.h"
#include "Visualization/Timeline.h"
#include "Visualization/EdgeFactory.h"
#include "GuidedLecture.h"

#include "InstanceManager.h"
#include "RegionGrowingComponent.h"

#include "FlexElements.generated.h"

class UEdgeFactory;
class ASimulationSnapshotActor;
class AMLRuleActor;

// To create a rigid body in flex:
// - create a FFLexRigidBodyObject on a node
// - connect the node to the particles that will make up the node with FFlexRigidBodyConnection
// - each of the connected node objects should have a FFlexParticleObject.
// - The FFlexRigidBodyObject should not have a FFlexParticleObject sibling. This will be nonsense and non-deterministic.
USTRUCT(BlueprintType)
struct LIFEBRUSH_API FFlexRigidBodyObject : public FGraphObject
{
	GENERATED_BODY()

public:
	// Internal index of the flex rigid body.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flex")
	int32 flexRigidIndex = 0;

	// The stiffness of the particles in the Flex rigid body, valid values in range [0,1].
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flex")
	float stiffness = 0.5f;

	// Plastic deformation threshold coefficients. Particles moving this distance past their rest position will be permanently
	// deformed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flex")
	float plasticDeformationThreshold = 2.0f;

	// The plastic deformation creep coefficient, it is valid in the range [0,1].
	// This is the rate of creep once a particle has passed its plasticDeformationThreshold.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flex")
	float plasticDeformationCreep;

	// Will create a flex rigid body, from the particlesInBody. A new FGraphNode, with a FFlexRigidBodyObject
	// will be created at the centre of mass, it will be connected to each of the particlesInBody with a FFlexRigidBodyConnection.
	// The returned node will be rotated and translated by flex, relative to the centre-of-mass of the rigid body.
	// Things like the rest positions are implicitly defined (computed once and passed to Flex, but not otherwise stored).
	// @param[in] particlesInBody Must contain two or more node handles, must not contain the rigidNode.
	// @return The rigidNode (if provided) otherwise a new node that has an attached FFlexRigidBodyObject. 
	//         The node will be reposition to the centre of mass of the rigid body.
	static FFlexRigidBodyObject& createRigidBody(FGraph& graph, TArray<FGraphNodeHandle>& particlesInBody, FGraphNodeHandle rigidNode = FGraphNodeHandle::null);

	static FGraphNodeHandle getRigidBodyHandle(FGraph& graph, FGraphNodeHandle subNodeHandle);

	void edgesAndNodes(FGraph& graph, TArray<FGraphNodeHandle>& nodes_out, TArray<FGraphEdgeHandle>& edges_out);

	// Finds the centre of mass, this doesn't write to anything.
	FVector calculateCenterOfMass(FGraph& graph);

	// Rotates the rigid body (and the linked nodes) around this node and then translates it.
	// @param[in] rotation The delta rotation to apply. 
	// @param[in] translation The delta translation to apply.
	// @param[in] graph The graph containing this object.
	void applyRotationTranslation(const FQuat& rotation, const FVector& translation, FGraph& graph);

	void applyRotationTranslationInferVelocity(const FQuat& rotation, const FVector& translation, FGraph& graph);

	void setRotationPosition(const FQuat rotation, const FVector position, FGraph& graph);

	void setRotationPositionInferVelocity(const FQuat rotation, const FVector position, FGraph& graph);

protected:
	static void _ensureFlexParticle(FGraphNode& node, FGraph& graph);
};

// Allows a node to be a member of a flex rigid object, but without a particle. The position of the node is updated
// relative to the position and orientation of the connected FFlexRigidBodyObject. Connect with a FFlexRigidBodyConnection.
USTRUCT(BlueprintType)
struct LIFEBRUSH_API FFlexRigidMember : public FGraphObject
{
	GENERATED_BODY()
public:
	// These are transients, computed at load or component-add
	FVector relativeOffset;
	FQuat relativeOrientation;

	int32 flexRigidIndex = 0;
};

// Added to each node in a rigid body, to store the original, base rotation of the nodes.
// The node.orientation = rigidBodyNode.orientation * node.component<FFlexBaseRotation>(graph).baseRotation
USTRUCT(BlueprintType)
struct LIFEBRUSH_API FFlexBaseRotation : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flex")
	FQuat rotation = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flex")
	int32 flexRigidBodyIndex;
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FFlexRigidBodyConnection : public FGraphEdgeObject
{
	GENERATED_BODY()

public:

};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FFlexConnection : public FGraphEdgeObject
{
	GENERATED_BODY()

public:
	FFlexConnection() {}

	FFlexConnection(float length, float coefficient) : length(length), coefficient(coefficient) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float length = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float coefficient = 0.5f;
};

UENUM( BlueprintType )
enum class EATPSynthaseState : uint8
{
	Spinning UMETA( DisplayName = "Active" ),
	Inactive UMETA( DisplayName = "Inactive" ),
};

UCLASS( BlueprintType )
class LIFEBRUSH_API UEvent_ProtonPumped : public USEGraphEvent
{
	GENERATED_BODY()

public:
	virtual ~UEvent_ProtonPumped() {}
};

// Makes an object for a three point path.
USTRUCT(BlueprintType)
struct LIFEBRUSH_API F3PointPathFollower : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FVector points[3];

	// The parameterized position along the path, in s.
	UPROPERTY()
	float time = 0.0f;

	// How long it takes to complete the path.
	UPROPERTY()
	float duration = 1.0f;

	UPROPERTY()
	int restoreChannel = 0;

	UPROPERTY()
	FVector terminalVelocity;
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FProtonPumpGraphObject : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float timerH = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float refractoryPeriodH = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	EATPSynthaseState state = EATPSynthaseState::Inactive;
};


USTRUCT( BlueprintType )
struct LIFEBRUSH_API FATPSynthaseGraphObject : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float timerH = -1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float refractoryPeriodH = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float timerADP = -1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float timerSpin = 0.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float timerSpinDuration = 2.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float refractoryPeriodADP = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float hyrogenInteractionRadius = 5.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	EATPSynthaseState state = EATPSynthaseState::Inactive;
};

USTRUCT( BlueprintType )
struct LIFEBRUSH_API FFlexParticleObject : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	int group = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	bool selfCollide = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	bool isFlud = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float inverseMass = 0.125f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	int channel = 0;
};

USTRUCT( BlueprintType )
struct LIFEBRUSH_API FHydrogenGraphObject : public FGraphObject
{
	GENERATED_BODY()

};

USTRUCT( BlueprintType )
struct LIFEBRUSH_API FADPGraphObject : public FGraphObject
{
	GENERATED_BODY()

};

USTRUCT( BlueprintType )
struct LIFEBRUSH_API FATPGraphObject : public FGraphObject
{
	GENERATED_BODY()

};

UCLASS(BlueprintType)
class LIFEBRUSH_API UEvent_SpawnATP : public USEGraphEvent
{
	GENERATED_BODY()

public:
	virtual ~UEvent_SpawnATP() {}
};

UCLASS(BlueprintType)
class LIFEBRUSH_API UEvent_SpinATPSynthase : public USEGraphEvent
{
	GENERATED_BODY()

public:
	virtual ~UEvent_SpinATPSynthase() {}
};



UCLASS( BlueprintType )
class LIFEBRUSH_API UATPSynthaseSimulation : public UObjectSimulation, public IFlexGraphSimulation
{
	GENERATED_BODY()
public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	TArray<FTimStructBox> atpTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float a_max = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float r_min = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float hyrogenInteractionRadius = 5.0f;

protected:
	virtual void attach() override;

public:
	virtual void tick( float deltaT ) override;

	virtual void flexTick( 
		float deltaT, 
		NvFlexVector<int>& neighbourIndices, 
		NvFlexVector<int>& neighbourCounts, 
		NvFlexVector<int>& apiToInternal, 
		NvFlexVector<int>& internalToAPI, 
		int maxParticles 
	) override;

	std::shared_ptr<tcodsMeshInterface> meshInterface;

	FTransform toWorld;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	int getNumberOfParticles(ECanvasExample ex);

protected:
	void _tickFollowPath(float deltaT);

	void _tickProtonPumps( float deltaT, NvFlexVector<int>& neighbourIndices, NvFlexVector<int>& neighbourCounts, NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI, int maxParticles );

	FGraphNode& _spawnATP( FVector position, FQuat orientation, float scale );

protected:
	float _g;	// g = a_max * r_min^2 / m_h. However, we just assume m_h is 1. We just want to approximate a_max at a certain distance.
};

UCLASS()
class LIFEBRUSH_API URandomWalkSimulation : public UObjectSimulation
{
	GENERATED_BODY()
protected:
	virtual void attach() override;

public:
	virtual void tick(float deltaT) override;

	std::shared_ptr<tcodsMeshInterface> meshInterface;

	FTransform toWorld;
};


USTRUCT( BlueprintType )
struct LIFEBRUSH_API FSpinnerGraphObject : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float angularVelocityMagnitude = 1.0f;
};


USTRUCT( BlueprintType )
struct LIFEBRUSH_API FVelocityGraphObject : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	FVector linearVelocity = FVector::ZeroVector;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	FVector angularVelocity = FVector::ZeroVector;
};

USTRUCT( BlueprintType )
struct LIFEBRUSH_API FStaticPositionObject : public FGraphObject
{
	GENERATED_BODY()

public:
	FStaticPositionObject() {}
	FStaticPositionObject(FVector position, FQuat orientation) : position(position), orientation(orientation), didLoad(true) {}

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	FVector position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	FQuat orientation = FQuat::Identity;

	bool didLoad = false;
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FStabalizedPosition : public FGraphObject
{
	GENERATED_BODY()

public:
	FStabalizedPosition() {}
	FStabalizedPosition(FVector position, FQuat orientation) : position(position), orientation(orientation), didLoad(true) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	FVector position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	FQuat orientation = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	float strength = 0.1f;

	bool didLoad = false;
};

USTRUCT( BlueprintType )
struct LIFEBRUSH_API FRandomWalkGraphObject : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float timeLeft = 0.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float maxVelocityOffset = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	float baseVelocity = 1.0f;
};

USTRUCT( BlueprintType )
struct LIFEBRUSH_API FSurfaceBoundGraphObject : public FGraphObject
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Mitochondria" )
	FQuat lastSurfaceRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mitochondria")
	FSurfaceIndex surfaceIndex;
};



UCLASS()
class LIFEBRUSH_API UStaticPositionSimulation : public UObjectSimulation
{
	GENERATED_BODY()
protected:
	virtual void attach();

public:
	virtual void tick(float deltaT) override;
	virtual void tick_paused(float deltaT) override;

	void _tickStabalized(float deltaT);
	void _tickStatic(float deltaT);

	FTransform toWorld;
};



/////////////////////////COVID SIMULATION///////////////////////////////////////////////
UENUM()
enum COVID_Status
{
	EHealthy,
	EInfected,
	EDead,
	EImmune,
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FCOVID_spike : public FGraphObject {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Spike Protein")
		float bindingRadius;
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FCOVID_ACE2 : public FGraphObject {
	GENERATED_BODY()

};

UCLASS()
class LIFEBRUSH_API UEvent_detachSpikeHead : public USEGraphEvent {
	GENERATED_BODY()
public:
	virtual ~UEvent_detachSpikeHead() {};
};

//represents a single agent in the community transmission simulation
USTRUCT(BlueprintType)
struct LIFEBRUSH_API FCOVIDSim_Agent : public FGraphObject {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "COVID Sim")
		TEnumAsByte<COVID_Status> status;

//number of other agents that this agent has infected
	float _numberInfected = 0;
	

};

UCLASS()
class LIFEBRUSH_API UCOVIDSim : public UObjectSimulation, public IFlexGraphSimulation
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "COVID Sim")
		UMaterialInstance* healthy_material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "COVID Sim")
		UMaterialInstance* infected_material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "COVID Sim")
		UMaterialInstance* immune_material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "COVID Sim")
		float interactionRadius;

	//prescribes the time interval steps of the simulation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "COVID Sim")
		float simulationTimer;

	//r naught value - 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "COVID Sim")
		float r_naught;

	//current number of healthy agents
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "COVID Sim")
		int num_healthyAgents;

	//current number of infected agents
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "COVID Sim")
		int num_infectedAgents;

	//current number of immune agents
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "COVID Sim")
		int num_immuneAgents;

	//current number of dead agents
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "COVID Sim")
		int num_deadAgents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "COVID Sim")
		int	num_totalAgents;



private:

	float _simTimer;

	//boolean is flipped when an agent transitions states
	bool _isDirty;

protected:
	virtual void attach() override;

	void _agentInteraction(FCOVIDSim_Agent* agent,FGraphNode* agent_node, FCOVIDSim_Agent* neighbour);
	void _detachSpike(FCOVID_spike* spike);
	//recalculates number of immune, infected etc. when change is made to model.
	void _recalculateTotals();		

public:
	virtual void tick(float deltaT) override;
	virtual void tick_paused(float deltaT) override;

	virtual void flexTick(
		float deltaT,
		NvFlexVector<int>& neighbourIndices,
		NvFlexVector<int>& neighbourCounts,
		NvFlexVector<int>& apiToInternal,
		NvFlexVector<int>& internalToAPI,
		int maxParticles
	)override;

};



//////////////////////////CENTRAL DOGMA SIMULATION//////////////////////////////////////


UCLASS(BlueprintType)
class LIFEBRUSH_API UEvent_spawnRNA : public USEGraphEvent
{
	GENERATED_BODY()

public:
	virtual ~UEvent_spawnRNA() {}
};

UCLASS(BlueprintType)
class LIFEBRUSH_API UEvent_SpawnProtein : public USEGraphEvent
{
	GENERATED_BODY()

public:
	virtual ~UEvent_SpawnProtein() {}
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FCentralDog_DNA_GraphObject : public FGraphObject
{
	GENERATED_BODY()
public:

	bool bIsGTFBound = false;
	bool bIsPolBound = false;
	bool bHasBeenTranscribed = false;

	bool isPolBound() { return bIsPolBound; }
	bool isGTFBound() { return bIsGTFBound; }
	bool hasBeenTranscribed() { return bHasBeenTranscribed; }
	bool isBeingTranscribed() { return isPolBound() && isGTFBound(); }

	void setHasBeenTranscribed(bool b) { bHasBeenTranscribed = b; }
	void setGTFBound(bool bound) { bIsGTFBound = bound; }
	void setPolBound(bool bound) { bIsPolBound = bound; }

	float transcriptionTimer;

	//used to store initial random walk values so we can restore them after DNA is unbound
	float defaultRandWalk_MaxOff;
	float defaultRandWalk_BaseVel;

	//references to bound GTF and Pol objects corresponding to a given DNA strand
	int32 boundGTFIndex;
	int32 boundPolIndex;

};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FCentralDog_RNA_GraphObject : public FGraphObject
{
	GENERATED_BODY()

public:
		
	FVector outerMembranePoint = FVector (-1.f, 103.f, 174.f);
	int32 boundRiboIndex = NULL;

	float translationTimer;

	float defaultRandWalk_MaxOff;
	float defaultRandWalk_BaseVel;
		
	bool bHasBeenTranslated = false;
	bool bIsRiboBound = false;

	bool isRiboBound() { return bIsRiboBound; }
	bool hasBeenTranslated() { return bHasBeenTranslated; }

	void setIsRiboBound(bool b) { bIsRiboBound = b; }
	void setHasBeenTranslated(bool b) { bHasBeenTranslated = b; }


	
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FCentralDog_Ribosome_GraphObject : public FGraphObject
{
	GENERATED_BODY()
public:

	

	bool bIsRNABound = false;

	bool isRNABound() { return bIsRNABound; }

	void setRNABound(bool b) { bIsRNABound = b; }
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FCentralDog_TranscriptFactors_GraphObject : public FGraphObject
{
	GENERATED_BODY()
public:

	bool bIsDNABound = false;

	bool isDNABound() { return bIsDNABound; }

	void setDNABound(bool bound) { bIsDNABound = bound; }


};



USTRUCT(BlueprintType)
struct LIFEBRUSH_API FCentralDog_Polymerase_GraphObject : public FGraphObject
{
	GENERATED_BODY()
public:
	bool bIsDNABound = false;

	bool isDNABound() { return bIsDNABound; }

	void setDNABound(bool bound) { bIsDNABound = bound; }
};



//Simulation showing transcription and translation of DNA
UCLASS()
class LIFEBRUSH_API UCentralDogmaSimulation : public UObjectSimulation, public IFlexGraphSimulation
{
	GENERATED_BODY()

public:


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Central Dogma")
	TArray<FTimStructBox> rnaTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Central Dogma")
		TArray<FTimStructBox> translatedProteinTemplate;
		
protected:
	virtual void attach() override;

public:
	virtual void tick(float deltaT) override;
	virtual void tick_paused(float deltaT) override;

	virtual void flexTick(
		float deltaT,
		NvFlexVector<int>& neighbourIndices,
		NvFlexVector<int>& neighbourCounts,
		NvFlexVector<int>& apiToInternal,
		NvFlexVector<int>& internalToAPI,
		int maxParticles
	)override;

protected:



	FGraphNode& _spawnRNA(FVector position, FQuat orientation, float scale);
	FGraphNode& _spawnProtein(FVector position, FQuat orientation, float scale);

	void bindGTFtoDNA(FCentralDog_DNA_GraphObject& dna, FCentralDog_TranscriptFactors_GraphObject& gtf);
	void bindPolToDNA(FCentralDog_DNA_GraphObject& dna, FCentralDog_Polymerase_GraphObject& rnaPol);
	void bindRiboToRNA(FCentralDog_RNA_GraphObject& rna, FCentralDog_Ribosome_GraphObject& ribo);

	void unbindRiboFromRNA(FCentralDog_RNA_GraphObject& rna);
	void unbindPolFromDNA(FCentralDog_DNA_GraphObject& dna);
	void unbindGTFfromDNA(FCentralDog_DNA_GraphObject& dna);
	

};



///////////////////////////////////////////////////////////////////////////////////////////

USTRUCT( BlueprintType )
struct FNvFlexParameters
{
	GENERATED_BODY()

public:
	//!< Number of solver iterations to perform per-substep
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	int numIterations = 3;					

	//!< Constant acceleration applied to all particles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	FVector gravity = FVector(0.0f,0.0f,-9.81f);	

	//!< The maximum interaction radius for particles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float radius = 1.0f;						

	//!< The distance non-fluid particles attempt to maintain from each other, must be in the range (0, radius]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float solidRestDistance = 0.0f;			

	//!< The distance fluid particles are spaced at the rest density, must be in the range (0, radius], for fluids this should generally be 50-70% of mRadius, for rigids this can simply be the same as the particle radius
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float fluidRestDistance = 0.5f;		

	// common params

	//!< Coefficient of friction used when colliding against shapesUPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float dynamicFriction = 0.0f;				
	//!< Coefficient of static friction used when colliding against shapes
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float staticFriction = 0.0f;				
	//!< Coefficient of friction used when colliding particles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float particleFriction = 0.0f;				
	//!< Coefficient of restitution used when colliding against shapes, particle collisions are always inelastic
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float restitution = 0.0f;					
	//!< Controls how strongly particles stick to surfaces they hit, default 0.0, range [0.0, +inf]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float adhesion = 0.0f;						
	//!< Particles with a velocity magnitude < this threshold will be considered fixed
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float sleepThreshold = 0.0f;				

	//!< The magnitude of particle velocity will be clamped to this value at the end of each step
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float maxSpeed = FLT_MAX;					
	//!< The magnitude of particle acceleration will be clamped to this value at the end of each step (limits max velocity change per-second), useful to avoid popping due to large interpenetrations
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float maxAcceleration = 100.0f;				

	//!< Artificially decrease the mass of particles based on height from a fixed reference point, this makes stacks and piles converge faster
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float shockPropagation = 0.0f;				
	//!< Damps particle velocity based on how many particle contacts it has
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float dissipation = 0.0f;					
	//!< Viscous drag force, applies a force proportional, and opposite to the particle velocity
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float damping = 0.0f;						

	// cloth params

	//!< Constant acceleration applied to particles that belong to dynamic triangles, drag needs to be > 0 for wind to affect triangles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	FVector wind = FVector::ZeroVector;						
	//!< Drag force applied to particles belonging to dynamic triangles, proportional to velocity^2*area in the negative velocity direction
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float drag  = 0.0f;				
	//!< Lift force applied to particles belonging to dynamic triangles, proportional to velocity^2*area in the direction perpendicular to velocity and (if possible), parallel to the plane normal
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float lift = 0.0f;							

	// fluid params

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float cohesion = 0.25f;						//!< Control how strongly particles hold each other together, default: 0.025, range [0.0, +inf]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float surfaceTension = 0.01f;				//!< Controls how strongly particles attempt to minimize surface area, default: 0.0, range: [0.0, +inf]    
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float viscosity = 0.1f;					//!< Smoothes particle velocities using XSPH viscosity
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float vorticityConfinement = 40.0f;			//!< Increases vorticity by applying rotational forces to particles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float anisotropyScale = 20.0f;				//!< Control how much anisotropy is present in resulting ellipsoids for rendering, if zero then anisotropy will not be calculated, see NvFlexGetAnisotropy()
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float anisotropyMin = 0.1f;				//!< Clamp the anisotropy scale to this fraction of the radius
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float anisotropyMax = 2.0f;				//!< Clamp the anisotropy scale to this fraction of the radius
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float smoothing = 1.0f;					//!< Control the strength of Laplacian smoothing in particles for rendering, if zero then smoothed positions will not be calculated, see NvFlexGetSmoothParticles()
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float solidPressure = 1.0f;				//!< Add pressure from solid surfaces to particles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float freeSurfaceDrag = 0.0f;				//!< Drag force applied to boundary fluid particles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float buoyancy = 1.0f;						//!< Gravity is scaled by this value for fluid particles

	// diffuse params

	//!< Particles with kinetic energy + divergence above this threshold will spawn new diffuse particles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float diffuseThreshold = 100.0f;				
	//!< Scales force opposing gravity that diffuse particles receive
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float diffuseBuoyancy = 1.0f;				
	//!< Scales force diffuse particles receive in direction of neighbor fluid particles
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float diffuseDrag = 0.8f;					
	//!< The number of neighbors below which a diffuse particle is considered ballistic
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	int diffuseBallistic = 16;				
	//!< Time in seconds that a diffuse particle will live for after being spawned, particles will be spawned with a random lifetime in the range [0, diffuseLifetime]
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float diffuseLifetime = 2.0f;				

	// collision params
	
	//!< Distance particles maintain against shapes, note that for robust collision against triangle meshes this distance should be greater than zero
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float collisionDistance = 0.0f;	
	//!< Increases the radius used during neighbor finding, this is useful if particles are expected to move significantly during a single step to ensure contacts aren't missed on subsequent iterations
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float particleCollisionMargin = 0.0f;			
	//!< Increases the radius used during contact finding against kinematic shapes
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float shapeCollisionMargin = 0.0f;			

	//!< Collision planes in the form ax + by + cz + d = 0
	UPROPERTY( EditAnywhere,  Category = "Flex" )
	FVector4 planes[8];					
	//!< Num collision planes
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	int numPlanes = 0;						

	//NvFlexRelaxationMode relaxationMode;//!< How the relaxation is applied inside the solver

	// 0 - eNvFlexRelaxationGlobal
	// 1 - eNvFlexRelaxationLocal
	//!< How the relaxation is applied inside the solver
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	int relaxationMode = 1;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Flex" )
	float relaxationFactor = 1.0f;	

	NvFlexParams toNvFlexParams()
	{
		NvFlexParams params;

		params.numIterations = numIterations;

		params.gravity[0] = gravity.X;
		params.gravity[1] = gravity.Y;
		params.gravity[2] = gravity.Z;
		
		params.radius = radius;
		params.solidRestDistance = solidRestDistance;
		params.fluidRestDistance = fluidRestDistance;

		// common params
		params.dynamicFriction = dynamicFriction;
		params.staticFriction = staticFriction;
		params.particleFriction = particleFriction;
		params.restitution = restitution;
		params.adhesion = adhesion;
		params.sleepThreshold = sleepThreshold;

		params.maxSpeed = maxSpeed;
		params.maxAcceleration = maxAcceleration;

		params.shockPropagation = shockPropagation;
		params.dissipation = dissipation;
		params.damping = damping;

		// cloth params
		params.wind[0] = wind.X;
		params.wind[1] = wind.Y;
		params.wind[2] = wind.Z;

		params.drag = drag;
		params.lift = lift;

		// fluid params
		params.cohesion = cohesion;
		params.surfaceTension = surfaceTension;
		params.viscosity = viscosity; 
		params.vorticityConfinement = vorticityConfinement;
		params.anisotropyScale = anisotropyScale;
		params.anisotropyMin = anisotropyMin;
		params.anisotropyMax = anisotropyMax;
		params.smoothing = smoothing;
		params.solidPressure = solidPressure;
		params.freeSurfaceDrag = freeSurfaceDrag;
		params.buoyancy = buoyancy;

		// diffuse params
		params.diffuseThreshold = diffuseThreshold;
		params.diffuseBuoyancy = diffuseBuoyancy;
		params.diffuseDrag = diffuseDrag;
		params.diffuseBallistic = diffuseBallistic;
		params.diffuseLifetime = diffuseLifetime;

		// collision params
		params.collisionDistance = collisionDistance;
		params.particleCollisionMargin = particleCollisionMargin;
		params.shapeCollisionMargin = shapeCollisionMargin;

		for( int i = 0; i < 8; ++i )
		{
			for(int j = 0; j < 4; ++j)
				params.planes[i][j] = planes[i][j];
		}
		
		params.numPlanes = numPlanes;

		params.relaxationMode = relaxationMode == 0 ? NvFlexRelaxationMode::eNvFlexRelaxationGlobal : NvFlexRelaxationMode::eNvFlexRelaxationLocal;
		params.relaxationFactor = relaxationFactor;

		return params;	
	}
};
















/*
  Interfaces a FGraph simulation with the Flex particle physics engine. It will be responsible for ticking the simulation manager.

  It will register two additional simulations:
  - UVisualization_AgentPathLines
  - URandomWalkSimulation
*/
struct FFlexSimulation : public ComponentListener, EdgeObjectListener
{
public:
	typedef std::function<void(
		float /*deltaT*/,
		NvFlexVector<int>& /*neighbourIndices*/,
		NvFlexVector<int>& /*neighbourCounts*/,
		NvFlexVector<int>& /*apiToInternal*/,
		NvFlexVector<int>& /*internalToAPI */,
		int /*maxParticles*/)>
		FlexTickWork_t;



public:


	FFlexSimulation(FGraph& graph, UGraphSimulationManager& simulationManager, FNvFlexParameters flexParams) :
		graphSimulation(graph),
		simulationManager(simulationManager),
		flexParams(flexParams)
	{
		_meshInterface = std::make_shared<tcodsMeshInterface>();
	}

	virtual ~FFlexSimulation()
	{
		uninit();
	}

	// Called before initMeshInterface
	auto initSimulationManager(AActor * owner) -> void;

	auto setCamera(UCameraComponent * camera) -> void;

	// Call this after init. It will send FObjectSimulation::begin to all the simulations,
	// this is the place to start showing graphics associated with the simulation,
	// outside of a FObjectSimulation::tick or FObjectSimulation::tick_paused.
	auto begin() -> void;

	auto uninit() -> void;

	auto tick(float deltaT) -> void;
	auto clear() -> void;
	auto pause() -> void;
	auto play() -> void;
	auto isPlaying() -> bool { return _playing; }

	auto updateSphereWorldSpace(FVector position, float radius) -> void;

	auto updateFlexState() -> void;
	auto exportElementDomain()->FGraphSnapshot;

	auto setInstanceManagerBounds(FBox instanceManagerRelativeBounds) -> void;
	auto instanceManagerBounds() -> FBox { return _bounds; }

	auto addTickWork(std::function<void()> work) -> void;
	auto addFlexTickWork(FlexTickWork_t work) -> void;

	std::shared_ptr<tcodsMeshInterface> meshInterface() { return _meshInterface; }

protected:
	auto _initFlex() -> void;


	auto _initParams() -> void;

	void _preTick(float DeltaTime);
	void _tickGraph(float DeltaTime);
	void _tickGraph_paused(float DeltaTime);
	void _flexTick(float DeltaTime);
	void _postTick(float DeltaTime);


	auto _instanceCount()->size_t;

	void _hackInitChannels();

	auto _spawnShapes(NvFlexCollisionGeometry * geometry, FVector4 * shapePositions, FQuat * shapeRotations, int * shapeFlags) -> void;
	auto _spawnMesh(NvFlexCollisionGeometry * geometry, FVector4 * shapePositions, FQuat * shapeRotations, int * shapeFlags) -> void;

	// The spring buffers should have already been mapped before calling.
	void _writeSpringState();


	void _loadRigids();
	void _readRigidRotations();

	auto _readFlexState(FVector4 * positions, FVector * velocities, int * phases) -> void;
	auto _writeFlexState(FVector4 * positions, FVector * velocities, int * phases, int * active) -> void;

	auto _integrateRotations(float deltaT) -> void;

	auto _loadTcodsMesh(tcodsMeshInterface& mesh) -> void;

	auto _loadMesh(UStaticMeshComponent * meshComponent) -> void;

	

protected: // ComponentListener
	virtual void componentAdded(FGraphNodeHandle node, ComponentType type) override;
	virtual void componentRemoved(FGraphNodeHandle node, ComponentType type) override;

	virtual void connectionAdded(int32 edgeIndex, FGraphEdge& edge, ComponentType type) override;
	virtual void connectionRemoved(int32 edgeIndex, FGraphEdge& oldEdge, ComponentType type) override;

	void flexRigidBodyObjectRemoved(FGraphNodeHandle node);

protected: // EdgeObjectListener
	virtual void edgeObjectAdded(FGraphEdgeHandle handle, EdgeObjectType type) override;
	virtual void edgeObjectRemoved(FGraphEdgeHandle handle, EdgeObjectType type) override;

public:
	FGraph & graphSimulation;
	UGraphSimulationManager& simulationManager;
	FNvFlexParameters flexParams;

	AActor * owner;
	UCameraComponent * _camera;

protected:
	NvFlexLibrary * _library = nullptr;

	std::vector< std::function<void()> > _tickWork;
	std::vector< FlexTickWork_t > _flexTickWork;

	FBox _bounds;

	NvFlexInitDesc _initDesc;
	NvFlexSolverDesc _solverDescription;
	NvFlexParams _params;

	NvFlexSolver * _solver = nullptr;

	NvFlexBuffer * _particleBuffer = nullptr;
	NvFlexBuffer * _velocityBuffer = nullptr;
	NvFlexBuffer * _phaseBuffer = nullptr;
	NvFlexBuffer * _activeBuffer = nullptr;

	NvFlexBuffer * _geometryBuffer = nullptr;
	NvFlexBuffer * _shapePositionsBuffer = nullptr;
	NvFlexBuffer * _shapeRotationsBuffer = nullptr;
	NvFlexBuffer * _shapeFlagsBuffer = nullptr;

	struct Springs
	{
		NvFlexVector<int> indices; // index pairs
		NvFlexVector<float> lengths; // rest length
		NvFlexVector<float> coefficients; // stiffness

		Springs(NvFlexLibrary * l) :
			indices(l),
			lengths(l),
			coefficients(l)
		{}

		void init()
		{
			indices.resize(0);
			lengths.resize(0);
			coefficients.resize(0);
		}

		void map()
		{
			indices.map();
			lengths.map();
			coefficients.map();
		}

		void unmap()
		{
			indices.unmap();
			lengths.unmap();
			coefficients.unmap();
		}
	};

	std::unique_ptr<Springs> _springs;

	struct Rigids
	{
		Rigids(NvFlexLibrary * l) :
			offsets(l),
			indices(l),
			localRestPositions(l),
			localRestNormals(l),
			stiffness(l),
			plasticThresholds(l),
			plasticCreeps(l),
			bodyRotations(l),
			bodyTranslations(l)
		{}

		// Pointer to a buffer of start offsets for a rigid in the indices array, should be numRigids+1 in length, the first entry must be 0
		// Flex uses offsets like this:
		// body_i start = offsets[i]
		// body_i end = offsets[i + 1] // exclusive
		// Therefore, offsets must be n + 1 in size, since Flex needs an end index for the last body!
		NvFlexVector<int> offsets;

		// Pointer to a buffer of indices for the rigid bodies, the indices for the jth rigid body start at indices[offsets[j]] and run to indices[offsets[j+1]] exclusive
		NvFlexVector<int> indices;

		// Pointer to a buffer of local space positions relative to the rigid's center of mass (average position), this should be at least numIndices in length in the format x,y,z
		NvFlexVector<FVector> localRestPositions;

		// Pointer to a buffer of local space normals, this should be at least numIndices in length in the format x, y, z, w where w is the (negative) signed distance of the particle inside its shape
		NvFlexVector<FVector4> localRestNormals;

		// Pointer to a buffer of rigid stiffness coefficents, should be numRigids in length, valid values in range [0, 1]
		NvFlexVector<float> stiffness;

		// Pointer to a buffer of plastic deformation threshold coefficients, should be numRigids in length
		NvFlexVector<float> plasticThresholds;

		// Pointer to a buffer of plastic deformation creep coefficients, should be numRigids in length, valid values in range [0, 1]
		NvFlexVector<float> plasticCreeps;

		// Pointer to a buffer of quaternions (numRigids in length)
		NvFlexVector<FQuat> bodyRotations; // Unreal's FQuat is <x,y,z,w> as is NVidia's Quat <x,y,z,w>. So this is fine.

		// Pointer to a buffer of translations of the center of mass (numRigids in length)
		NvFlexVector<FVector> bodyTranslations;
		
		//NvFlexVector<int> rigidMeshSize;


		void init()
		{
			offsets.resize(0);
			indices.resize(0);
			stiffness.resize(0);
			plasticThresholds.resize(0);
			plasticCreeps.resize(0);
			bodyRotations.resize(0);
			bodyTranslations.resize(0);
			localRestPositions.resize(0);
			localRestNormals.resize(0);
		}

		// maps any buffers that haven't been mapped yet
		void mapRemaining()
		{
			if( !offsets.mappedPtr ) offsets.map();
			if( !indices.mappedPtr ) indices.map();
			if( !stiffness.mappedPtr ) stiffness.map();
			if( !plasticThresholds.mappedPtr ) plasticThresholds.map();
			if( !plasticCreeps.mappedPtr ) plasticCreeps.map();
			if( !bodyRotations.mappedPtr ) bodyRotations.map();
			if( !bodyTranslations.mappedPtr ) bodyTranslations.map();
			if( !localRestPositions.mappedPtr ) localRestPositions.map();
			if( !localRestNormals.mappedPtr ) localRestNormals.map();
		}

		void mapAll()
		{
			offsets.map();
			indices.map();
			stiffness.map();
			plasticThresholds.map();
			plasticCreeps.map();
			bodyRotations.map();
			bodyTranslations.map();
			localRestPositions.map();
			localRestNormals.map();
		}

		void unmapAll()
		{
			offsets.unmap();
			indices.unmap();
			stiffness.unmap();
			plasticThresholds.unmap();
			plasticCreeps.unmap();
			bodyRotations.unmap();
			bodyTranslations.unmap();
			localRestPositions.unmap();
			localRestNormals.unmap();
		}

		void mapRotationsTranslations()
		{
			bodyRotations.map();
			bodyTranslations.map();
		}

		void unmapRotationsTranslations()
		{
			bodyRotations.unmap();
			bodyTranslations.unmap();
		}
		void reset()
		{
			offsets.resize(0);
			indices.resize(0);
			stiffness.resize(0);
			plasticThresholds.resize(0);
			plasticCreeps.resize(0);
			bodyRotations.resize(0);
			bodyTranslations.resize(0);
			localRestPositions.resize(0);
			localRestNormals.resize(0);
		}
	};

	std::unique_ptr<Rigids> _rigids;

	std::unique_ptr< NvFlexVector<int> > _neighborsIndicesBuffer;
	std::unique_ptr< NvFlexVector<int> > _neighborsCountsBuffer;
	std::unique_ptr< NvFlexVector<int> > _neighborsAPIToInternal;
	std::unique_ptr< NvFlexVector<int> > _neighborsInternalToAPI;



	std::unique_ptr< NvFlexVector<FVector4> > _mesh0_positions;
	std::unique_ptr< NvFlexVector<int> > _mesh0_indices;
	NvFlexTriangleMeshId _mesh0_Id;
	FBox _mesh0_bounds;

	bool _needsFlexReset = false;
	bool _didInitFlex = false;
	bool _playing = false;

	bool _didInitSImulationManager = false;

	InstanceManager _instanceManager;
	bool _didSpawnShapesAndMesh = false;

	FVector _spherePosition = FVector(0.0f,0.0f,-500.0f);
	float _sphereRadius = 0.0f;

	const size_t _maxParticles = 34000;
	const size_t _maxNeighbors = 64;

	size_t nParticles = 0;

	const size_t nGeometries = 2;

	std::shared_ptr<tcodsMeshInterface> _meshInterface;
	FTransform _meshToWorld;

	// We don't have fine enough control over the flex buffers to insert/remove rigids, we'll just reconstruct the whole thing.
	// Maybe NvFlexSetRigids(..., indices, ...) is where we can insert some dead indices? make them inactive? I don't know. Reconstructing
	// the whole flex state isn't that expensive generally.
	bool _flexRigidsDirty = true; 
	std::set<FGraphNodeHandle> _flexRigidsToRemove;

	ComponentType _FFlexParticleObjectType;
	ComponentType _FFlexRigidObjectType;
};











UCLASS(ClassGroup = (Custom), DefaultToInstanced, meta = (BlueprintSpawnableComponent))
class LIFEBRUSH_API UGraphComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY( BlueprintReadWrite, Category = "Mitochondria" )
	FGraph graph;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Mitochondria")
	UGraphSimulationManager * simulationManager;

public:
	UGraphComponent();

	virtual void InitializeComponent();

	virtual void initGraph();

protected:
	bool _didInitGraph = false;
};









UCLASS( ClassGroup = (Custom), DefaultToInstanced, meta = (BlueprintSpawnableComponent) )
class LIFEBRUSH_API UFlexSimulationComponent : public UGraphComponent
{
	GENERATED_BODY()

public:


public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "LifeBrush" )
	FNvFlexParameters flexParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	AActor * rulesActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	FSoftObjectPath softMLRulesActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	AActor * swarmGrammarRulesActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	FSoftObjectPath softSwarmGrammarRulesActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	UMaterialInterface * meshInterfaceMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	FBox limits = FBox(EForceInit::ForceInitToZero);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	bool drawLimits = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	float timeStep = 1.0f / 90.0f;

	// To allow this component to tick the FFlexSimulation struct, set this to true.
	// This can be used to allow another object (like the URegionGrowingGeneration) to control FFlexSimulation ticking.
	// This is not the same thing as FFlexSimulation:isPlaying(). 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	bool tickFlexSimulation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LifeBrush")
	TArray<AActor*> otherSceneActors;

protected:
	std::unique_ptr<FFlexSimulation> _flexSimulation;

	TArray<UStaticMeshComponent*> _meshesForMeshInterface;

	UPROPERTY()
	URuntimeMeshComponent * _meshInterfaceRMC = nullptr;

	std::vector<uint32_t> _chunkSections;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UBoxComponent * _limitsBoxComponent = nullptr;

	std::unique_ptr<SynthesisContext> _context;

	bool _didInitOnce = false;

public:
	UFUNCTION(BlueprintCallable, Category = LifeBrush)
	void playSimulation();

	UFUNCTION(BlueprintCallable, Category = LifeBrush)
	void pauseSimulation();

	UFUNCTION(BlueprintCallable, Category = LifeBrush)
	void initWithCameraComponent(UCameraComponent * camera);

	UFUNCTION(BlueprintCallable, Category = LifeBrush)
	bool isSimulationPlaying();

public:
	UFlexSimulationComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void BeginDestroy() override;

	virtual void InitializeComponent();

	virtual void BeginPlay();

	auto init(FTransform meshInterfaceToWorld, UCameraComponent * camera) -> void;

	void updateMeshInterface(class UChunkedVolumeComponent * chunkVolume);

	FFlexSimulation* flexSimulation() { return _flexSimulation.get(); }
	SynthesisContext * context() { return _context.get(); }

	// Creates a snapshot of the geometry created for the simulation. Call snapshotToActor on each simulation.
	ASimulationSnapshotActor* createGraphicalSnapshotActor(UWorld * world);

	// Duplicates this component into a new actor, owned by world. Includes the simulation manager and graph. It doesn't duplicate
	// transient components. It doesn't call createSnapshotActor.
	AActor * snapshotSimulationStateToWorld(UWorld * world);

	URuntimeMeshComponent * meshInterfaceRMC() { return _meshInterfaceRMC; }


protected:
	void initFlexSimulationObject();

	void _initContext();
	void _initMeshInterface(std::shared_ptr<tcodsMeshInterface> meshInterface);
	void _cacheMeshesForMeshInterface();

	void _readMLRules();
	void _readSGRules();

	// Limits Drawing
// --------------
	void _updateLimits();
	void _updateDrawLimits();
	void _drawLimits();
	void _hideLimits();
};

