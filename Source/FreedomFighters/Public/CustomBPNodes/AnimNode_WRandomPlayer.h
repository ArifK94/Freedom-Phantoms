// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Math/RandomStream.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_WRandomPlayer.generated.h"

struct FWRandomAnimPlayData
{
	FWRandomAnimPlayData()
		: InternalTimeAccumulator(0.0f)
		, PlayRate(1.0f)
	{

	}



	// Current time through the sequence
	float InternalTimeAccumulator;

	// Calculated play rate 
	float PlayRate;

	// Marker tick record for this play through
	FMarkerTickRecord MarkerTickRecord;
};

/** The random player node holds a list of sequences and parameter ranges which will be played continuously
  * In a random order. If shuffle mode is enabled then each entry will be played once before repeating any
  */
USTRUCT(BlueprintInternalUseOnly)
struct FWRandomPlayerSequenceEntry
{
	GENERATED_BODY()

		FWRandomPlayerSequenceEntry()
		: Sequence(nullptr)
		, ChanceToPlay(1.0f)
		, MinPlayRate(1.0f)
		, MaxPlayRate(1.0f)
	{

	}

	/** Sequence to play when this entry is picked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		UAnimSequence* Sequence;

	/** When not in shuffle mode, this is the chance this entry will play (normalized against all other sample chances) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		float ChanceToPlay;

	/** Minimum play rate for this entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (UIMin = "0", ClampMin = "0"))
		float MinPlayRate;

	/** Maximum play rate for this entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (UIMin = "0", ClampMin = "0"))
		float MaxPlayRate;

};

USTRUCT(BlueprintInternalUseOnly)
struct FREEDOMFIGHTERS_API FAnimNode_WRandomPlayer : public FAnimNode_Base
{
	GENERATED_BODY()

		FAnimNode_WRandomPlayer();

public:

	/** List of sequences to randomly step through */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		TArray<FWRandomPlayerSequenceEntry> Entries;

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface

private:

	int32 GetNextEntryIndex();

	// Normalized list of play chances when we aren't using shuffle mode
	TArray<float> NormalizedPlayChances;

	// The currently playing entry in the entries list
	int32 CurrentEntry;


	// Index of the 'current' data set in the PlayData array
	int32 CurrentDataIndex;

	// Play data for the current and next sequence
	TArray<FWRandomAnimPlayData> PlayData;

	// Random number source
	FRandomStream RandomStream;
};