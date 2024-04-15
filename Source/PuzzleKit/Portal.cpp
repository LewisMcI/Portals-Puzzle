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
}
bool done = false;
double timeToBegin = 0;
bool complete = false;
void APortal::Tick(float DeltaTime)
{
	if (!done) {
		done = true;
		timeToBegin = UGameplayStatics::GetTimeSeconds(GetWorld()) + 0.01f;
	}
	Super::Tick(DeltaTime);
	if (done && !complete && (UGameplayStatics::GetTimeSeconds(GetWorld()) > timeToBegin))
	{
		BeginVisuals();
		complete = true;
	}
	if (complete && !hidden)
	{
		//SceneCaptureUpdateRecursive(FVector::ZeroVector, FRotator::ZeroRotator);
		CheckIfMoved();
		UpdateSceneCapture();
		CheckViewportSize();
		ShouldTeleport();
	}
}

void APortal::UpdateSceneCapture()
{
	FVector newLocation = SceneCaptureUpdateLocation();
	FRotator newRotation = SceneCaptureUpdateRotation();

	otherPortal->portalCamera->SetWorldLocationAndRotation(newLocation, newRotation);
}

