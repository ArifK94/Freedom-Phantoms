// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomBPNodes/AnimNode_WRandomPlayer.h"

#include "AnimationRuntime.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimSequence.h"


FAnimNode_WRandomPlayer::FAnimNode_WRandomPlayer()
	: CurrentEntry(INDEX_NONE)
	, CurrentDataIndex(0)
{

}

void FAnimNode_WRandomPlayer::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
	GetEvaluateGraphExposedInputs().Execute(Context);

	const int32 NumEntries = Entries.Num();

	if (NumEntries == 0)
	{
		// early out here, no need to do anything at all if we're not playing anything
		return;
	}

	NormalizedPlayChances.Empty(NormalizedPlayChances.Num());
	NormalizedPlayChances.AddUninitialized(NumEntries);

	// Initialize normalized play chance for each entry and validate entry data
	float SumChances = 0.0f;
	for (FWRandomPlayerSequenceEntry& Entry : Entries)
	{
		SumChances += Entry.ChanceToPlay;
	}

	for (int32 Idx = 0; Idx < NumEntries; ++Idx)
	{
		NormalizedPlayChances[Idx] = Entries[Idx].ChanceToPlay / SumChances;
	}

	// Initialize random stream and pick first entry
	RandomStream.Initialize(FPlatformTime::Cycles());

	CurrentEntry = GetNextEntryIndex();

	PlayData.Empty(2);
	PlayData.AddDefaulted(2);

	FWRandomAnimPlayData& CurrentData = PlayData[CurrentDataIndex];


	// Init play data

	CurrentData.PlayRate = RandomStream.FRandRange(Entries[CurrentEntry].MinPlayRate, Entries[CurrentEntry].MaxPlayRate);

}

void FAnimNode_WRandomPlayer::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);

	if (Entries.Num() == 0)
	{
		// We don't have any entries, play data will be invalid - early out
		return;
	}

	FWRandomAnimPlayData* CurrentData = &PlayData[CurrentDataIndex];


	if (UAnimSequence* CurrentSequence = Entries[CurrentEntry].Sequence)
	{
		if (FAnimInstanceProxy* AnimProxy = Context.AnimInstanceProxy)
		{
			FAnimGroupInstance* SyncGroup;
			FAnimTickRecord& TickRecord = AnimProxy->CreateUninitializedTickRecord(INDEX_NONE, SyncGroup);
			AnimProxy->MakeSequenceTickRecord(TickRecord, Entries[CurrentEntry].Sequence, false, CurrentData->PlayRate, Context.GetFinalBlendWeight(), CurrentData->InternalTimeAccumulator, CurrentData->MarkerTickRecord);


		}
	}
}

void FAnimNode_WRandomPlayer::Evaluate_AnyThread(FPoseContext& Output)
{
	if (Entries.Num() > 0)
	{
		UAnimSequence* CurrentSequence = Entries[CurrentEntry].Sequence;

		if (CurrentSequence)
		{
			FWRandomAnimPlayData& CurrentData = PlayData[CurrentDataIndex];
			{
				// Single animation
				CurrentSequence->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(CurrentData.InternalTimeAccumulator, Output.AnimInstanceProxy->ShouldExtractRootMotion()));
			}
		}
		else
		{
			Output.ResetToRefPose();
		}
	}
	else
	{
		Output.ResetToRefPose();
	}
}

void FAnimNode_WRandomPlayer::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugData.AddDebugItem(DebugLine, true);
}

int32 FAnimNode_WRandomPlayer::GetNextEntryIndex()
{
	if (Entries.Num() > 0)
	{
		{
			float RandomVal = RandomStream.GetFraction();
			const int32 NumEntries = Entries.Num();

			// Grab the entry index corresponding to the value
			for (int32 Idx = 0; Idx < NumEntries; ++Idx)
			{
				RandomVal -= NormalizedPlayChances[Idx];
				if (RandomVal <= 0.0f)
				{
					return Idx;
				}
			}
		}
	}

	return INDEX_NONE;
}