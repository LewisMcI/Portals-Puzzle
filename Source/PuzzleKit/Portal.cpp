#include "Portal.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

	Setup();
}

void APortal::BeginPlay()
{
	UKismetSystemLibrary::Delay(GetWorld(), 0.1f, FLatentActionInfo());
	Super::BeginPlay();
	PrimaryActorTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
	BeginVisuals();
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//SceneCaptureUpdateRecursive(FVector::ZeroVector, FRotator::ZeroRotator);
	UpdateSceneCapture();
	CheckViewportSize();
	ShouldTeleport();
}

void APortal::UpdateSceneCapture()
{
	FVector newLocation = SceneCaptureUpdateLocation();
	FRotator newRotation = SceneCaptureUpdateRotation();

	otherPortal->portalCamera->SetWorldLocationAndRotation(newLocation, newRotation);
}

