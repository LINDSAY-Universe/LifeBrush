// Copyright 2019, Timothy Davison. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElementActor.h"
#include "GuidedLecture.generated.h"


UENUM(BlueprintType)
enum class EControllerEmphasis : uint8
{
	EGrip,
	ETrig,
	EAbutton,
	EBbutton,
	Ethumb,
	
};

UENUM(BlueprintType)
enum class EUIEmphasis : uint8
{
	ECanvas,
	EPalette,
	ENametags,
	EMitoMatrix,
};

UENUM(BlueprintType)
enum class ECanvasExample : uint8
{
	EADP,
	EHydrogen,
	EIMS,
	EInMembrane,
	ELipids,
	EJunction,
	EMatrix,
	ESynthase,
	EAll,
};

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FLectureSlide
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FText textBody;

	//The Element Actors in each slides palette will be invisible until that slide is reached
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AElementActor*> slidePalette;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* displayImage;

};


USTRUCT(BlueprintType)
struct LIFEBRUSH_API FPaletteSlide
{
	GENERATED_USTRUCT_BODY()

public:

	//The Element Actors in each slides palette will be invisible until that slide is reached
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AElementActor*> slidePalette;

};

UCLASS()
class LIFEBRUSH_API AGuidedLecture : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGuidedLecture();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<FLectureSlide> slideSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int startingSlide = 0;

	//Where we will store the highlighted words and their definitions
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FString> vocabDictionary;

	//Parsing function which will fill the vocabDictionary
	UFUNCTION(BlueprintCallable)
	void BuildVocabDictionary(TArray<FLectureSlide> inSlide);
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};

UCLASS()
class LIFEBRUSH_API ATutorialLecture : public AActor
{
	GENERATED_BODY()

public:
	ATutorialLecture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* exemplar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* completedCanvas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	APawn* SketchyPawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* ADP_Snap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AActor* Hydrogen_Snap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* IMS_Snap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* inMembrane_Snap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* Junction_Snap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* Matrix_Snap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* Synthase_Snap;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AActor* Simulation_Actor;

	TArray<AActor*> snapshots;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetVisibilityOfAllSnapshots(bool b);
};

UCLASS()
class LIFEBRUSH_API ATemplateLecture : public AActor
{
	GENERATED_BODY()

public:
	ATemplateLecture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPaletteSlide> slidePalette;
	

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};


