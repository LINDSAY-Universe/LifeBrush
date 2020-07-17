// Copyright 2019, Timothy Davison. All rights reserved.

#include "LifeBrush.h"
#include "GuidedLecture.h"


// Sets default values
AGuidedLecture::AGuidedLecture()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AGuidedLecture::BuildVocabDictionary(TArray<FLectureSlide> inSlide)
{


	for (FLectureSlide slide : inSlide) {

		FString temp_string = slide.textBody.ToString();
		bool wordsRemaining = true;

		if (!temp_string.Contains("{", ESearchCase::IgnoreCase)) {
			return;
		}

		FRegexPattern paragraphPattern("{.*?}");    //captures {key:definition}
		FRegexPattern keyPattern("/.*:/g");				 //captures {key:
		FRegexPattern defPattern("//g");				//captures :definiton}

		FRegexMatcher paragraphMatcher(paragraphPattern, temp_string);
		

		int32 matchIndex = 0;

		while (wordsRemaining) {

			//captures definition snippet
			FString snippet = paragraphMatcher.GetCaptureGroup(matchIndex);

			FRegexMatcher keyMatcher(keyPattern, snippet);
			FString keyText = keyMatcher.GetCaptureGroup(0);

			FRegexMatcher defMatcher(defPattern, snippet);
			FString defText = defMatcher.GetCaptureGroup(0);

			//check if key has already been added to dictionary
			if (vocabDictionary.Contains(keyText)) {
				continue;
			}
			else {
				//else add key and definition to dictionary
				vocabDictionary.Add(keyText, defText);

			}


			if (matchIndex == paragraphMatcher.GetEndLimit()) {
				wordsRemaining = !wordsRemaining;
			}

			matchIndex++;


		}

	}

	
	

}

// Called when the game starts or when spawned
void AGuidedLecture::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGuidedLecture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


AInfoPane :: AInfoPane()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AInfoPane::BeginPlay()
{
	Super::BeginPlay();
}

void AInfoPane::Tick(float dt)
{
	Super::Tick(dt);
}


// Sets default values
ATutorialLecture::ATutorialLecture()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATutorialLecture::BeginPlay()
{

	snapshots.Add(ADP_Snap);
	snapshots.Add(Hydrogen_Snap);
	snapshots.Add(IMS_Snap);
	snapshots.Add(inMembrane_Snap);
	snapshots.Add(Junction_Snap);
	snapshots.Add(Matrix_Snap);
	snapshots.Add(Synthase_Snap);

	Super::BeginPlay();

}

// Called every frame
void ATutorialLecture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATutorialLecture::SetVisibilityOfAllSnapshots(bool b)
{
	for (AActor* snap : snapshots)
	{
		snap->SetActorHiddenInGame(b);
	}
}


ATemplateLecture::ATemplateLecture()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATemplateLecture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void ATemplateLecture::BeginPlay()
{
	Super::BeginPlay();
}

