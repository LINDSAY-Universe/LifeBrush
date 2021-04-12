#include "LifeBrush.h"
#include "ImmuneSysSimulation.h"


#include "Mac/CocoaThread.h"
#include "Simulation/FlexElements.h"

void UImmuneSysSimulation::attach()
{
}

void UImmuneSysSimulation::tick(float deltaT)
{
}

void UImmuneSysSimulation::flexTick(float deltaT, NvFlexVector<int>& neighbourIndices,
	NvFlexVector<int>& neighbourCounts, NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI,
	int maxParticles)
{

	auto& naiveBCells = graph->componentStorage<FImmune_NaiveB>();
	auto& helperTCells = graph->componentStorage<FImmune_HelperT>();
	auto& pathogens = graph->componentStorage<FImmune_Pathogen>();
	auto& macrophages = graph->componentStorage<FImmune_Macrophage>();
	
	graph->beginTransaction();

	NaiveBCellBehavior(naiveBCells,helperTCells,neighbourIndices,
		neighbourCounts,apiToInternal,internalToAPI,maxParticles);

	HelperTBehaviour(helperTCells,naiveBCells,macrophages,neighbourIndices,neighbourCounts,
		apiToInternal,internalToAPI,maxParticles);

	MacrophageBehavior(macrophages,pathogens,neighbourIndices,neighbourCounts,
        apiToInternal,internalToAPI,maxParticles);


	graph->endTransaction();
}

//MACROPHAGE FUNCTION
void UImmuneSysSimulation::MacrophageBehavior(TypedComponentStorage<FImmune_Macrophage> macrophages, TypedComponentStorage<FImmune_Pathogen> pathogens,
	NvFlexVector<int>& neighbourIndices, NvFlexVector<int>& neighbourCounts, NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToApi, int maxParticles)
{
	for(FImmune_Macrophage macro : macrophages)
	{
		if(!macro.isValid())
			continue;

		switch(macro.MacrophageState)
		{
			case EImmune_MacrophageState::MacroFree:
				MacroFreeState(macro,pathogens,neighbourIndices,neighbourCounts,apiToInternal,internalToApi,maxParticles);
				break;
			case EImmune_MacrophageState::MacroPathogenBound:
				MacroBoundState(macro,neighbourIndices,neighbourCounts,apiToInternal,internalToApi,maxParticles);
				break;
			default:
				break;
				
		}
	}
}


void UImmuneSysSimulation::MacroBoundState(FImmune_Macrophage macro, NvFlexVector<int>& neighbourIndices,
	NvFlexVector<int>& neighbourCounts, NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToApi, int maxParticles)
{
	//transitions when activated T-Cell comes near.
	//destroys pathogen
	FGraphNode& macroGraphNode = graph->node(macro.nodeIndex);

	if(!macroGraphNode.hasComponent<FFlexParticleObject>())
		return;

	if(!macroGraphNode.hasComponent<FStabalizedPosition>())
	{
		FStabalizedPosition stabPos = macroGraphNode.component<FStabalizedPosition>(*graph);
		stabPos.position = macroGraphNode.position;
		stabPos.strength = 5.0f;
	}
	const int nodeIndex_flexInternal = apiToInternal[macro.nodeIndex];
	const int neighbourCount = neighbourCounts[nodeIndex_flexInternal];

	for(int i = 0 ; i < neighbourCount; ++i)
	{
		const int neighbourIndex = internalToApi[neighbourIndices[i*maxParticles + nodeIndex_flexInternal]];

		FGraphNodeHandle neighbourHandle(neighbourIndex);
		FGraphNode neighbourGraphNode = graph->node(neighbourHandle);

		if(!neighbourGraphNode.hasComponent<FImmune_HelperT>())
			continue;

		FImmune_HelperT tCell = neighbourGraphNode.component<FImmune_HelperT>(*graph);

		const float distSqrd = (neighbourGraphNode.position - macroGraphNode.position).SizeSquared();

		if(distSqrd < macro.bindingRadius)
		{
			macroGraphNode.removeComponent<FStabalizedPosition>(*graph);

			if(!macroGraphNode.hasComponent<FRandomWalkGraphObject>())
				macroGraphNode.addComponent<FRandomWalkGraphObject>(*graph);
			FRandomWalkGraphObject randWalk = macroGraphNode.component<FRandomWalkGraphObject>(*graph);
			randWalk.baseVelocity = macro.baseVelocity;
			randWalk.maxVelocityOffset = macro.maxOffset;

			macro.MacrophageState = EImmune_MacrophageState::MacroFree;

			//kill the pathogen
			auto pathogenComponents = neighbourGraphNode.components;
			for(ComponentType comp : pathogenComponents)
				neighbourGraphNode.removeComponent(*graph,comp);
		}
	}
	
}

void UImmuneSysSimulation::MacroFreeState(FImmune_Macrophage macro, TypedComponentStorage<FImmune_Pathogen> pathogens,
	NvFlexVector<int>& neighbourIndices, NvFlexVector<int>& neighbourCounts, NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToApi, int maxParticles)
{
	FGraphNode& macroGraphNode = graph->node(macro.nodeIndex);

	if(!macroGraphNode.hasComponent<FFlexParticleObject>())
		return;

	if(!macroGraphNode.hasComponent<FRandomWalkGraphObject>())
	{
		FRandomWalkGraphObject randWalk = macroGraphNode.addComponent<FRandomWalkGraphObject>(*graph);
		randWalk.baseVelocity = macro.baseVelocity;
		randWalk.maxVelocityOffset = macro.maxOffset;
	}

	const int nodeIndex_flexInternal = apiToInternal[macro.nodeIndex];
	const int neighbourCount = neighbourCounts[nodeIndex_flexInternal];

	for(int i = 0 ; i < neighbourCount; ++i)
	{
		const int neighbourIndex = internalToApi[neighbourIndices[i*maxParticles + nodeIndex_flexInternal]];

		FGraphNodeHandle neighbourHandle(neighbourIndex);
		FGraphNode neighbourGraphNode = graph->node(neighbourHandle);

		if(!neighbourGraphNode.hasComponent<FImmune_Pathogen>())
			continue;

		FImmune_Pathogen pathogen =  neighbourGraphNode.component<FImmune_Pathogen>(*graph);

		FVector vecToPathogen = macroGraphNode.position - neighbourGraphNode.position;

		const float distSqrd = vecToPathogen.SizeSquared();

		if(distSqrd > macro.interactionRadius)
			continue;

		if(!macroGraphNode.hasComponent<FVelocityGraphObject>())
			macroGraphNode.addComponent<FVelocityGraphObject>(*graph);
		FVelocityGraphObject& velObj = macroGraphNode.component<FVelocityGraphObject>(*graph);
		velObj.linearVelocity = vecToPathogen;

		if(distSqrd < macro.bindingRadius)
		{
			FRandomWalkGraphObject randWalk = macroGraphNode.component<FRandomWalkGraphObject>(*graph);
			macroGraphNode.removeComponent<FRandomWalkGraphObject>(*graph);
			macroGraphNode.removeComponent<FVelocityGraphObject>(*graph);


			macro.MacrophageState = EImmune_MacrophageState::MacroPathogenBound;
			

			if(!macroGraphNode.hasComponent<FStabalizedPosition>())
				macroGraphNode.addComponent<FStabalizedPosition>(*graph);

			FStabalizedPosition stabPos = macroGraphNode.component<FStabalizedPosition>(*graph);
			stabPos.position = neighbourGraphNode.position;
			stabPos.strength = 5.0f;
		}
	}
}






//HELPER T CELL FUNCTIONS
void UImmuneSysSimulation::HelperTBehaviour(TypedComponentStorage<FImmune_HelperT>TCells,
    TypedComponentStorage<FImmune_NaiveB> BCells,TypedComponentStorage<FImmune_Macrophage> macrophages,
    NvFlexVector<int>& neighbourIndices, NvFlexVector<int>& neighbourCounts,
    NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToApi, int maxParticles)
{


	for(FImmune_HelperT tCell : TCells)
	{
		if(!tCell.isValid())
			continue;
		
		switch (tCell.TCellState)
		{
			case EImmune_TCellState::SeekingMacrophage:
				TCellSeekingMacrophage(tCell,macrophages,neighbourIndices,neighbourCounts,apiToInternal,internalToApi,maxParticles);
				break;

			case EImmune_TCellState::Activated:
				TCellActivated(tCell,BCells,neighbourIndices,neighbourCounts,apiToInternal,internalToApi,maxParticles);
				break;

			case EImmune_TCellState::BCellBound:
				TCellBCellBound(tCell);
				break;

			default:
				break;
			
		}
		
	}
	


}

void UImmuneSysSimulation::TCellSeekingMacrophage(FImmune_HelperT tCell, TypedComponentStorage<FImmune_Macrophage> Macrophages,
        NvFlexVector<int>& neighbourIndices, NvFlexVector<int>& neighbourCounts,
        NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI, int maxParticles )
{

	FGraphNode& tCellGraphNode = graph->node(tCell.nodeIndex);

	if(!tCellGraphNode.hasComponent<FFlexParticleObject>())
		return;

	if(!tCellGraphNode.hasComponent<FRandomWalkGraphObject>())
	{
		FRandomWalkGraphObject randWalk = tCellGraphNode.addComponent<FRandomWalkGraphObject>(*graph);
		randWalk.baseVelocity = tCell.baseVelocity;
		randWalk.maxVelocityOffset = tCell.maxOffset;
	}
	
	const int nodeIndex_flexInternal = apiToInternal[tCell.nodeIndex];
	const int neighbourCount = neighbourCounts[nodeIndex_flexInternal];

	for(int i = 0; i < neighbourCount; ++i)
	{
		const int neighbourIndex = internalToAPI[neighbourIndices[i*maxParticles + nodeIndex_flexInternal]];

		FGraphNodeHandle neighbourHandle(neighbourIndex);
		FGraphNode& NeighbourGraphNode = graph->node(neighbourHandle);

		//if not a macrophage, we are not interested
		
		
		if(!NeighbourGraphNode.hasComponent<FImmune_Macrophage>())
			continue;

		FImmune_Macrophage& macro = NeighbourGraphNode.component<FImmune_Macrophage>(*graph);

		if(macro.MacrophageState == EImmune_MacrophageState::MacroFree)
			continue;

		
		FVector vecToNeighbour = tCellGraphNode.position - NeighbourGraphNode.position;
		
		const float distSqrd = vecToNeighbour.SizeSquared();

		if(distSqrd > tCell.interactionRadius)
			continue;

		if(!tCellGraphNode.hasComponent<FVelocityGraphObject>())
			tCellGraphNode.addComponent<FVelocityGraphObject>(*graph);
		FVelocityGraphObject& velObj = tCellGraphNode.component<FVelocityGraphObject>(*graph);
		velObj.linearVelocity = vecToNeighbour;

		if(distSqrd < tCell.bindingRadius)
		{
			tCell.boundMacro = &NeighbourGraphNode;
				//if bound stop moving and change state
			tCellGraphNode.removeComponent<FVelocityGraphObject>(*graph);
			
			tCell.TCellState = EImmune_TCellState::Activated;
			macro.MacrophageState = EImmune_MacrophageState::MacroFree;
		}
	}

	
}


void UImmuneSysSimulation::TCellBCellBound(FImmune_HelperT tCell)
{
	FGraphNode& tCellGraphNode = graph->node(tCell.nodeIndex);

	//remove position stabalizer and add random  motion back
	if(tCellGraphNode.hasComponent<FStabalizedPosition>())
		tCellGraphNode.removeComponent<FStabalizedPosition>(*graph);

	if(!tCellGraphNode.hasComponent<FRandomWalkGraphObject>())
		tCellGraphNode.addComponent<FRandomWalkGraphObject>(*graph);
	FRandomWalkGraphObject randWalk = tCellGraphNode.component<FRandomWalkGraphObject>(*graph);
	randWalk.baseVelocity = tCell.baseVelocity;
	randWalk.maxVelocityOffset = tCell.maxOffset;
	

	
	tCell.TCellState = EImmune_TCellState::SeekingMacrophage;
	
}


void UImmuneSysSimulation::TCellActivated(FImmune_HelperT tCell, TypedComponentStorage<FImmune_NaiveB> BCells,
        NvFlexVector<int>& neighbourIndices, NvFlexVector<int>& neighbourCounts,
        NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI, int maxParticles)
{
	FGraphNode& tCellGraphNode = graph->node(tCell.nodeIndex);

	if(!tCellGraphNode.hasComponent<FFlexParticleObject>())
		return;;
	
	const int nodeIndex_flexInternal = apiToInternal[tCell.nodeIndex];
	const int neighbourCount = neighbourCounts[nodeIndex_flexInternal];

	for(int i = 0; i < neighbourCount; ++i)
	{
		const int neighbourIndex = internalToAPI[neighbourIndices[i*maxParticles + nodeIndex_flexInternal]];

		FGraphNodeHandle neighbourHandle(neighbourIndex);
		FGraphNode& NeighbourGraphNode = graph->node(neighbourHandle);

		//if not a macrophage, we are not interested
		if(!NeighbourGraphNode.hasComponent<FImmune_NaiveB>())
			continue;
		
		FVector vecToNeighbour = tCellGraphNode.position - NeighbourGraphNode.position;
		
		const float distSqrd = vecToNeighbour.SizeSquared();

		if(distSqrd > tCell.interactionRadius)
			continue;

		if(!tCellGraphNode.hasComponent<FVelocityGraphObject>())
			tCellGraphNode.addComponent<FVelocityGraphObject>(*graph);
		FVelocityGraphObject& velObj = tCellGraphNode.component<FVelocityGraphObject>(*graph);
		velObj.linearVelocity = vecToNeighbour;
		
		if(distSqrd < tCell.bindingRadius)
		{
			tCell.boundBCell = &NeighbourGraphNode;

			if(tCellGraphNode.hasComponent<FVelocityGraphObject>())
			{
				tCellGraphNode.removeComponent<FVelocityGraphObject>(*graph);
				//if bound stop moving and change state
			}

			tCell.TCellState = EImmune_TCellState::BCellBound;

			if(!tCellGraphNode.hasComponent<FStabalizedPosition>())
				tCellGraphNode.addComponent<FStabalizedPosition>(*graph);
			FStabalizedPosition stablePos = tCellGraphNode.component<FStabalizedPosition>(*graph);
			stablePos.position = NeighbourGraphNode.position;
			stablePos.strength = 5.0f;
			
		}

		
	}
}




FGraphNode& UImmuneSysSimulation::SpawnMolecule(FVector pos, FQuat orient, TArray<FTimStructBox> structBox)
{
	UE_LOG(LogTemp,Warning,TEXT("Spawn Mol"));
	FGraphNode& node = graph->node(graph->addNode(pos,orient,.05f));

	for(FTimStructBox& box : structBox)
	{
		if(!box.IsValid())
			continue;

		ComponentType type = FGraphObject::componentType(box.scriptStruct);

		FGraphObject* object = node.addComponent(*graph,type);

		box.scriptStruct->CopyScriptStruct(object,box.structMemory);

		object->nodeIndex = node.id;
	}

	return node;
}



//NAIVE B CELL FUNCTIONS///
void UImmuneSysSimulation::NaiveBCellBehavior(TypedComponentStorage<FImmune_NaiveB> bCells,
    TypedComponentStorage<FImmune_HelperT> TCells,NvFlexVector<int>& neighbourIndices,
    NvFlexVector<int>& neighbourCounts, NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI,
    int maxParticles) 
{


	for(FImmune_NaiveB bCell : bCells)
	{
		if(!bCell.isValid())
			continue;

		switch(bCell.BCellState)
		{
			case EImmune_BCellState::Unbound:
				BCellUnBoundState(bCell,TCells,neighbourIndices,neighbourCounts,apiToInternal,internalToAPI,maxParticles);
				break;
			case EImmune_BCellState::TCellBound:
				BCellTCellBoundState(bCell,TCells,neighbourIndices,neighbourCounts,apiToInternal,internalToAPI,maxParticles);
			default:
				break;
				
			
		}
	}
	


}


void UImmuneSysSimulation::BCellUnBoundState(FImmune_NaiveB bCell, TypedComponentStorage<FImmune_HelperT> TCells, NvFlexVector<int>& neighbourIndices,
    NvFlexVector<int>& neighbourCounts, NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI,
    int maxParticles)
{
	FGraphNode& bCellGraphNode = graph->node(bCell.nodeIndex);

	if(!bCellGraphNode.hasComponent<FFlexParticleObject>())
		return;

	if(!bCellGraphNode.hasComponent<FRandomWalkGraphObject>())
	{
		FRandomWalkGraphObject randWalk = bCellGraphNode.addComponent<FRandomWalkGraphObject>(*graph);
		randWalk.baseVelocity = bCell.baseVelocity;
		randWalk.maxVelocityOffset = bCell.maxOffset;
	}
		
	const int nodeIndex_flexInternal = apiToInternal[bCell.nodeIndex];
	const int neighbourCount = neighbourCounts[nodeIndex_flexInternal];

	for(int i = 0; i < neighbourCount; ++i)
	{
		const int neighbourIndex = internalToAPI[neighbourIndices[i*maxParticles + nodeIndex_flexInternal]];

		FGraphNodeHandle neighbourHandle(neighbourIndex);
		FGraphNode& neighbourGraphNode = graph->node(neighbourHandle);

		if(!neighbourGraphNode.hasComponent<FImmune_HelperT>())
			continue;

		FImmune_HelperT& tCellNeighbour = neighbourGraphNode.component<FImmune_HelperT>(*graph);

		if(tCellNeighbour.TCellState != EImmune_TCellState::Activated)
			continue;


		FVector vecToNeighbour = bCellGraphNode.position - neighbourGraphNode.position;

		const float distSqrd = vecToNeighbour.SizeSquared();

		if(distSqrd > bCell.interactionRadius)
			continue;

		if(!bCellGraphNode.hasComponent<FVelocityGraphObject>())
			bCellGraphNode.addComponent<FVelocityGraphObject>(*graph);
		FVelocityGraphObject& velObj = bCellGraphNode.component<FVelocityGraphObject>(*graph);
		velObj.linearVelocity = vecToNeighbour;

		if(distSqrd < bCell.bindingRadius)
		{

			bCellGraphNode.removeComponent<FRandomWalkGraphObject>(*graph);
			bCellGraphNode.removeComponent<FVelocityGraphObject>(*graph);
			
			bCell.BCellState = EImmune_BCellState::TCellBound;

			if(!bCellGraphNode.hasComponent<FStabalizedPosition>())
				bCellGraphNode.addComponent<FStabalizedPosition>(*graph);
			FStabalizedPosition stabPos = bCellGraphNode.component<FStabalizedPosition>(*graph);
			stabPos.position = neighbourGraphNode.position;
			stabPos.strength = 5.0f;
		}
		
	}
}


//TODO - 
void UImmuneSysSimulation::BCellTCellBoundState(FImmune_NaiveB bCell, TypedComponentStorage<FImmune_HelperT> TCells, NvFlexVector<int>& neighbourIndices,
    NvFlexVector<int>& neighbourCounts, NvFlexVector<int>& apiToInternal, NvFlexVector<int>& internalToAPI,
    int maxParticles)
{
/*
	FGraphNode& bCellGraphNode = graph->node(bCell.nodeIndex);
	//spawn shit
	FGraphNode& spawnedMemCell = graph->node(graph->addNode(bCellGraphNode.position,bCellGraphNode.orientation,bCellGraphNode.scale));
	for(FTimStructBox& box : bCell.memCellTemplate)
	{
		if(!box.IsValid())
			continue;

		ComponentType type = FGraphObject::componentType(box.scriptStruct);

		FGraphObject* obj = spawnedMemCell.addComponent(*graph,type);

		box.scriptStruct->CopyScriptStruct(obj,box.structMemory);

		obj->nodeIndex = bCellGraphNode.id;
	}

	FGraphNode& spawnedPlasmaCell = graph->node(graph->addNode(bCellGraphNode.position,bCellGraphNode.orientation,bCellGraphNode.scale));
	for(FTimStructBox& box : bCell.plasmaCellTemplate)
	{
		if(!box.IsValid())
			continue;

		ComponentType type = FGraphObject::componentType(box.scriptStruct);

		FGraphObject* obj = spawnedMemCell.addComponent(*graph,type);

		box.scriptStruct->CopyScriptStruct(obj,box.structMemory);

		obj->nodeIndex = bCellGraphNode.id;
	}
	//unbind
	if(bCellGraphNode.hasComponent<FStabalizedPosition>())
		bCellGraphNode.removeComponent<FStabalizedPosition>(*graph);
	

	//transition to unbound state
	bCell.BCellState = EImmune_BCellState::Unbound;
	*/
}


void UImmuneSysSimulation::BCellEngulfPathogen(FGraphNode& bCellNode, FGraphNode& pathNode)
{

	
	
	//first get both to stop moving
	if(bCellNode.hasComponent<FVelocityGraphObject>())
	{
		FVelocityGraphObject velObj = bCellNode.component<FVelocityGraphObject>(*graph);
		velObj.invalidate();
	}

	if(pathNode.hasComponent<FRandomWalkGraphObject>())
	{
		FRandomWalkGraphObject randObj = pathNode.component<FRandomWalkGraphObject>(*graph);
		randObj.invalidate();
	}

	
}

void UImmuneSysSimulation::BindMolecules(FGraphNode& molA, FGraphNode& molB)
{
	if(molA.hasComponent<FVelocityGraphObject>())
	{
		FVelocityGraphObject velObj = molA.component<FVelocityGraphObject>(*graph);
		velObj.invalidate();
	}

	if(molB.hasComponent<FVelocityGraphObject>())
	{
		FVelocityGraphObject velObj = molB.component<FVelocityGraphObject>(*graph);
		velObj.invalidate();
	}

	if(molA.hasComponent<FRandomWalkGraphObject>())
	{
		FRandomWalkGraphObject randObj = molA.component<FRandomWalkGraphObject>(*graph);
		randObj.invalidate();
	}

	if(molB.hasComponent<FRandomWalkGraphObject>())
	{
		FRandomWalkGraphObject randObj = molB.component<FRandomWalkGraphObject>(*graph);
		randObj.invalidate();
	}

	
}

void UImmuneSysSimulation::UnBindMolecules(FGraphNode&, FGraphNode&)
{
	
}





