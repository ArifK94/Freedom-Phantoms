// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomBPNodes/AnimNode_WRandomPlayerEditor.h"

#include "Animation/AnimInstanceProxy.h"
#include "CustomBPNodes/AnimNode_WRandomPlayerEditor.h"
#ifdef WITH_EDITOR
#include "EditorCategoryUtils.h"
#endif


#define LOCTEXT_NAMESPACE "AnimGraphNode_RandomPlayerNoLoop"

FLinearColor UAnimNode_WRandomPlayerEditor::GetNodeTitleColor() const
{
	return FLinearColor(0.10f, 0.60f, 0.12f);
}

FText UAnimNode_WRandomPlayerEditor::GetTooltipText() const
{
	return LOCTEXT("NodeToolTip", "Plays sequences picked from a provided list in random orders without looping");
}

FText UAnimNode_WRandomPlayerEditor::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Random Sequence Player no Loop");
}

FText UAnimNode_WRandomPlayerEditor::GetMenuCategory() const
{
	return FEditorCategoryUtils::GetCommonCategory(FCommonEditorCategory::Animation);
}

#undef LOCTEXT_NAMESPACE