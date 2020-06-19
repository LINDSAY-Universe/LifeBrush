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

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FLectureSlide
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FText textBody;

	//The palette that will be available to user on a given slide
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AElementActor*> slidePalette;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* displayImage;

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
	bool isExemplarVisible;

	//References to the exemplar actors that we will toggle invisibility
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AElementActor*> exemplarActors;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	APawn* SketchyPawn;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};


