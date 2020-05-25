// Copyright 2019, Timothy Davison. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GuidedLecture.generated.h"

USTRUCT(BlueprintType)
struct LIFEBRUSH_API FLectureSlide
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere,BlueprintReadWrite, meta = (MultiLine = true))
	FText textBody;
	
	FTexture image;

	
	

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

