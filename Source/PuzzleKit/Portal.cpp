#include "Portal.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

	Setup();
}

void APortal::BeginPlay()
{
	Super::BeginPlay();
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
	BeginVisuals();
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSceneCapture();
	CheckViewportSize();
}

void APortal::UpdateSceneCapture()
{
	FVector newLocation = SceneCaptureUpdateLocation();
	FRotator newRotation = SceneCaptureUpdateRotation();

	otherPortal->portalCamera->SetWorldLocationAndRotation(newLocation, newRotation);

}

