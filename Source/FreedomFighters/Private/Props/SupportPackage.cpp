// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/SupportPackage.h"

ASupportPackage::ASupportPackage()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ASupportPackage::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASupportPackage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

