// Copyright 2017 Timothy Davison, all rights reserved.

#include "LifeBrush.h"

#include "RegionGrowingComponent.h"
#include "WidgetComponent.h"
#include "DigTool.h"
#include "VolumeComponent.h"
#include "ElementEditor/DiscreteElementEditorComponent.h"
#include <Kismet/KismetMathLibrary.h>


void UDigTool::init(FRGC_UToolInitProperties& initProperties, UCameraComponent * camera)
{
	UTool::init(initProperties);

	regionGrowingComponent = initProperties.regionGrowingComponent;
	editorComponent = initProperties.editor;

	_kernel = _gaussianKernel( 5 );
	_camera = camera;
}

void UDigTool::oneHandStart( UPrimitiveComponent * hand )
{
	_createSelectionMeshComponent( hand );
}

void UDigTool::oneHandEnd( UPrimitiveComponent * hand )
{
	_destroySelectionMeshComponent();
}

float UDigTool::_brushRadius()
{
	return (radius * selectionATriggerValue() * 0.9f) + 0.1f;
}

void UDigTool::tickOneHand( float dt, UPrimitiveComponent * hand, FTransform lastTransform )
{
	if(_selectionMeshComponent)
	{
		_selectionMeshComponent->SetRelativeScale3D( FVector( _brushRadius() * selectionMeshScaleFactor ) );
	}

	UWorld * world = hand->GetWorld();

	//TArray<FOverlapResult> overlaps;

	//FCollisionShape collisionShape = FCollisionShape::MakeSphere( 20.0f );

	//FCollisionQueryParams params;
	//params.bTraceComplex = false; // we want convex collision, not per triangle hits

	//world->OverlapMultiByChannel( overlaps,
	//	hand->GetComponentLocation(),
	//	hand->GetComponentRotation().Quaternion(),
	//	ECC_WorldStatic,
	//	collisionShape,
	//	params );
	//
	//for (FOverlapResult& result : overlaps)
	//{
	//	AActor * actor = result.GetActor();

	//	UChunkedVolumeComponent * volume = actor->FindComponentByClass<UChunkedVolumeComponent>();


	// HACK, for now, we'll dig everywhere

	for (TObjectIterator<UChunkedVolumeComponent> iterator; iterator; ++iterator )
	{
		UChunkedVolumeComponent * volume = *iterator;

		if(!volume || volume->GetWorld() != editorComponent->GetWorld())
			continue;

		FTransform transform = volume->GetOwner()->GetRootComponent()->GetComponentToWorld();

		FVector volumePosition = transform.InverseTransformPosition( hand->GetComponentLocation() );

		float transformScale = transform.GetMaximumAxisScale();
		transformScale = FMath::IsNearlyZero( transformScale ) ? 1.0f : transformScale;

		float volumeRadius = _brushRadius() / transformScale;

		if(_digMode == EDigMode::Adding || _digMode == EDigMode::Removing)
			_dig( volume, volumePosition, volumeRadius, dt );
		else if(_digMode == EDigMode::Smoothing)
			_smooth( volume, volumePosition, volumeRadius );

		_lastVolume = volume;
	}
}

void UDigTool::focused()
{
	_hideWidgets();
	_loadWidgets();

	_notifyDidModeDelegates( _digMode, _digMode );
	_notifyDigToolDelegates();

	// show all the volumes
	for (TObjectIterator<UChunkedVolumeComponent> iterator; iterator; ++iterator)
	{
		UChunkedVolumeComponent * volume = *iterator;

		if (!volume || volume->GetWorld() != editorComponent->GetWorld())
			continue;

		URuntimeMeshComponent * rmc = volume->GetOwner()->FindComponentByClass<URuntimeMeshComponent>();

		if (rmc)
			rmc->SetVisibility(true);
	}

}

void UDigTool::loseFocus()
{
	Super::loseFocus();

	if (popupActor)
	{
		popupActor->Destroy();
		popupActor = nullptr;
	}
}

void UDigTool::faceDown_released()
{
	if (!developerMode)
		return;

	editorComponent->updateMeshInterface(_lastVolume);
}



void UDigTool::faceUp_released( USceneComponent * interactionPoint /* = nullptr */ )
{
	//setDigMode( EDigMode::Smoothing );

	if (!developerMode)
		return;

	// save a mesh to an actor for each chunk
	for (TObjectIterator<UChunkedVolumeComponent> iterator; iterator; ++iterator)
	{
		UChunkedVolumeComponent * volume = *iterator;

		if (!volume || volume->GetWorld() != editorComponent->GetWorld())
			continue;

		URuntimeMeshComponent * oldRMC = volume->GetOwner()->FindComponentByClass<URuntimeMeshComponent>();

		if (!oldRMC)
			continue;;

		FTransform transform = volume->GetOwner()->GetRootComponent()->GetComponentToWorld();

		FVector volumePosition = transform.InverseTransformPosition(interactionPoint->GetComponentLocation());

		AActor * newActor = GetWorld()->SpawnActor<AActor>();

		{
			URuntimeMeshComponent * trimmedRMC = NewObject<URuntimeMeshComponent>(newActor);

			trimmedRMC->SetShouldSerializeMeshData(true);

			tcodsChunkedGridMeshInterface tempGridInterface;

			tempGridInterface.buildMesh(volume->grid(), transform, volume->isoLevel, editorComponent->context->limits, trimmedRMC);

			newActor->AddInstanceComponent(trimmedRMC);

			trimmedRMC->RegisterComponent();
		}

		{
			URuntimeMeshComponent * fullRMC = NewObject<URuntimeMeshComponent>(newActor);

			fullRMC->SetRuntimeMesh(oldRMC->GetRuntimeMesh());
			fullRMC->SetShouldSerializeMeshData(true);

			newActor->AddInstanceComponent(fullRMC);

			fullRMC->RegisterComponent();
		}
	}


}

void UDigTool::faceLeft_released()
{
	setDigMode( EDigMode::Adding );
}

void UDigTool::faceRight_released()
{
	setDigMode( EDigMode::Removing );
}

void UDigTool::_notifyDidModeDelegates( EDigMode newMode, EDigMode oldMode )
{
	// pad widget
	if( widgetComponent )
	{
		UUserWidget * widget = widgetComponent->GetUserWidgetObject();

		if(!widget || !widget->Implements<UDigToolDelegate>())
			return;

		IDigToolDelegate::Execute_didChangeDigMode( widget, newMode, oldMode );
	}

	// selection point widget
	if(selectionPointWidgetComponent) 
	{
		UUserWidget * widget = selectionPointWidgetComponent->GetUserWidgetObject();

		if(!widget || !widget->Implements<UDigToolDelegate>())
			return;

		IDigToolDelegate::Execute_didChangeDigMode( widget, newMode, oldMode );
	}
}

void UDigTool::_notifyDigToolDelegates()
{
	// pad widget
	if(widgetComponent)
	{
		UUserWidget * widget = widgetComponent->GetUserWidgetObject();

		if(!widget || !widget->Implements<UDigToolDelegate>())
			return;

		IDigToolDelegate::Execute_setDigTool( widget, this );
	}

	// selection point widget
	if(selectionPointWidgetComponent)
	{
		UUserWidget * widget = selectionPointWidgetComponent->GetUserWidgetObject();

		if(!widget || !widget->Implements<UDigToolDelegate>())
			return;

		IDigToolDelegate::Execute_setDigTool( widget, this );
	}
}

void UDigTool::setDigMode( EDigMode mode )
{
	auto oldMode = _digMode;

	_digMode = mode;

	_notifyDidModeDelegates( _digMode, oldMode );
}

bool UDigTool::consume_rightShoulder_pressed()
{
	if (!popupActorClass ) return false;

	UWorld * world = _camera->GetWorld();

	const FVector cameraLocation = _camera->GetComponentLocation();
	const FVector handLocation = _rightSelectionPoint->GetComponentLocation();

	FRotator rotator = UKismetMathLibrary::FindLookAtRotation(handLocation, cameraLocation);

	FTransform transform(rotator.Quaternion(), handLocation, FVector(1.0f));

	// only maintain one popupActor
	if (popupActor)
	{
		popupActor->Destroy();
		popupActor = nullptr;
	}

	popupActor = world->SpawnActor<ADigToolPopupActor>(popupActorClass, transform);
	popupActor->digTool = this;

	return true;
}

void UDigTool::SaveMeshToCollection()
{
	if (!_lastVolume) return;
	
	URuntimeMeshComponent * rmc = _lastVolume->GetOwner()->FindComponentByClass<URuntimeMeshComponent>();

#if WITH_EDITOR
	UWorld * world = GEditor->EditorWorld;

	AActor * newActor = world->SpawnActor<AActor>();

	// create scene root
	if( USceneComponent * newScene = NewObject<USceneComponent>(newActor) )
	{
		AActor * originalActor = rmc->GetOwner();

		USceneComponent * originalScene = originalActor->GetRootComponent();

		FTransform newTransform = originalScene->GetComponentTransform();

		newActor->SetRootComponent(newScene);
		newActor->AddInstanceComponent(newScene);

		newScene->SetWorldTransform(newTransform);
	}

	URuntimeMeshComponent * newRMC = Utility::duplicateRuntimeMeshComponentToActor(rmc, newActor);

	FString baseName = "chunkedMesh_";

	FString name = _actorLabelByDate(baseName);

	newActor->SetActorLabel(name);
#endif

	ExitSession();
}

FString UDigTool::_actorLabelByDate(FString baseName)
{
	FDateTime now = FDateTime::Now();

	return baseName += now.ToString();
}

void UDigTool::SaveMeshToScene()
{
	editorComponent->updateMeshInterface(_lastVolume);

	if (popupActor)
	{
		popupActor->Destroy();
		popupActor = nullptr;

		if (toolDelegate) toolDelegate->cedeFocus(this);
	}
}

void UDigTool::ExitSession()
{
	if (popupActor)
	{
		popupActor->Destroy();
		popupActor = nullptr;

		if (toolDelegate) toolDelegate->cedeFocus(this);
	}

	if (_lastVolume)
	{
		URuntimeMeshComponent * rmc = _lastVolume->GetOwner()->FindComponentByClass<URuntimeMeshComponent>();

		if (rmc)
		{
			rmc->SetVisibility(false);
		}
	}
}

void UDigTool::_createSelectionMeshComponent( UPrimitiveComponent * selectionPoint )
{
	if(_selectionMeshComponent)
		return;

	_selectionMeshComponent = NewObject<UStaticMeshComponent>( selectionPoint );
	_selectionMeshComponent->AttachToComponent( selectionPoint, FAttachmentTransformRules::KeepRelativeTransform );
	_selectionMeshComponent->SetStaticMesh( selectionMesh );
	_selectionMeshComponent->SetMaterial( 0, selectionMeshMaterial );
	_selectionMeshComponent->SetRelativeScale3D( FVector( _brushRadius() * selectionMeshScaleFactor ) );


	_selectionMeshComponent->RegisterComponent();
}

void UDigTool::_destroySelectionMeshComponent()
{
	if(_selectionMeshComponent == nullptr || !_selectionMeshComponent->IsValidLowLevel())
		return;

	_selectionMeshComponent->DestroyComponent();

	_selectionMeshComponent = nullptr;
}

void UDigTool::_smooth( UChunkedVolumeComponent * volume, FVector volumePosition, float volumeRadius )
{
	ChunkGrid<float>& grid = volume->grid();

	FVector cellSize = grid.cellSize();

	FVector extents = cellSize * volumeRadius;

	FIntVector start = volume->componentToIndex( volumePosition - extents );
	FIntVector end = volume->componentToIndex( volumePosition + extents );

	FIntVector index = start;

	const float rSqrd = volumeRadius * volumeRadius;

	// visit the cells
	for(index.Z = start.Z; index.Z <= end.Z; index.Z++)
	{
		for(index.Y = start.Y; index.Y <= end.Y; index.Y++)
		{
			for(index.X = start.X; index.X <= end.X; index.X++)
			{
				FVector p = grid.samplePoint( index );

				float d = FVector::DistSquared( p, volumePosition );

				if(d > rSqrd)
					continue;

				_convolve( grid, index );
			}
		}
	}

	FVector startP = volume->indexToComponent( start );
	FVector endP = volume->indexToComponent( end );

	volume->markDirty( startP - 2, endP + 2 );
}

void UDigTool::_convolve(ChunkGrid<float>& grid, FIntVector& index )
{
	float sum = 0.0f;

	FIntVector j;

	FIntVector dimensions = _kernel.dimensions();
	FIntVector base = dimensions / 2; // offset
	base += index; // from index

	// convolve with the kernel
	for(j.Z = 0; j.Z < dimensions.Z; j.Z++)
	{
		for(j.Y = 0; j.Y < dimensions.Y; j.Y++)
		{
			for(j.X = 0; j.X < dimensions.X; j.X++)
			{
				FIntVector j_ = base + j;

				sum += _kernel( j ) * grid( j_ );
			}
		}
	}

	grid.set( index, sum );
}

// Adapted from here
// https://stackoverflow.com/a/8204867
lb::BasicGrid<float> UDigTool::_gaussianKernel( size_t n )
{
	lb::BasicGrid<float> kernel;

	kernel.init(n, n, n);

	float sigma = 1.0f;
	float mean = n / 2.0f;

	float sum = 0.0f;

	for(size_t zi = 0; zi < n; ++zi)
	{
		for(size_t yi = 0; yi < n; ++yi)
		{
			for(size_t xi = 0; xi < n; ++xi)
			{
				float k = std::exp( -.5f * (
					std::pow( (xi - mean) / sigma, 2.0 ) +
					std::pow( (yi - mean) / sigma, 2.0 ) +
					std::pow( (zi - mean) / sigma, 2.0 )) 
				) / (2.0f * M_PI * sigma * sigma);

				kernel( xi, yi, zi ) = k;

				sum += k;
			}
		}
	}

	// normalize
	for(size_t zi = 0; zi < n; ++zi)
	{
		for(size_t yi = 0; yi < n; ++yi)
		{
			for(size_t xi = 0; xi < n; ++xi)
			{
				kernel( xi, yi, zi ) /= sum;
			}
		}
	}

	return kernel;
}

void UDigTool::_dig( UChunkedVolumeComponent * volume, FVector volumePosition, float volumeRadius, float dt )
{
	ChunkGrid<float>& grid = volume->grid();

	FVector cellSize = grid.cellSize();

	FVector extents = cellSize * volumeRadius;

	FIntVector start = volume->componentToIndex( volumePosition - extents );
	FIntVector end = volume->componentToIndex( volumePosition + extents );

	FIntVector index = start;

	const float rSqrd = volumeRadius * volumeRadius;

	float newGridValue = _digMode == EDigMode::Adding ? maxValue : -maxValue;

	for( index.Z = start.Z; index.Z <= end.Z; index.Z++)
	{
		for( index.Y = start.Y; index.Y <= end.Y; index.Y++)
		{
			for( index.X = start.X; index.X <= end.X; index.X++)
			{
				FVector p = grid.samplePoint( index );

				float d = FVector::DistSquared( p, volumePosition );

				if(d > rSqrd)
					continue; 

				float delta = (1.0f - (std::sqrt(d) / volumeRadius)) * newGridValue * dt * fillRate;

				// accumulate
				float gridValue = grid(index);

				gridValue += delta;

				if (gridValue > maxValue) gridValue = maxValue;
				if (gridValue < 0.0f) gridValue = 0.0f;

				grid.set(index, gridValue);
			}
		}
	}

	volume->markDirtyIndex(start - FIntVector(1), end + FIntVector(1));
}
