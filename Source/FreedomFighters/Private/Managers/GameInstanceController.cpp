// Fill out your copyright notice in the Description page of Project Settings.


#include "Managers/GameInstanceController.h"

#include "FreedomFighters/FreedomFighters.h"

#include "Particles/ParticleSystem.h"




UParticleSystem* UGameInstanceController::CheckSurface(EPhysicalSurface SurfaceType)
{
	switch (SurfaceType)
	{
	case SURFACE_HEAD:
	case SURFACE_GROIN:
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		return FleshImpactEffect;
		break;
	default:
		return DefaultImpactEffect;
		break;
	}
}