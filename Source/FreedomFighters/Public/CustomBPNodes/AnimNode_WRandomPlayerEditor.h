// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UObject/ObjectMacros.h"
#include "Math/RandomStream.h"
#include "Kismet2/BlueprintEditorUtils.h"

#include "AnimGraphNode_Base.h"
#include "CustomBPNodes/AnimNode_WRandomPlayer.h"
#include "AnimNode_WRandomPlayerEditor.generated.h"


UCLASS(MinimalAPI)
class UAnimNode_WRandomPlayerEditor : public UAnimGraphNode_Base
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = Settings)
		FAnimNode_WRandomPlayer Node;

	// UEdGraphNode interface
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetMenuCategory() const override;
	// End of UEdGraphNode interface
};