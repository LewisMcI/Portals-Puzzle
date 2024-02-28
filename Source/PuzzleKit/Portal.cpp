// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"
#include <Kismet/GameplayStatics.h>

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	Setup();
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();

	portalVisuals->BeginPlay(portalMat, otherPortal->portalCamera);
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSceneCapture();
}

void APortal::UpdateSceneCapture()
{
	// GetActorTransform
	FTransform portalTransform = GetActorTransform();
	// Set Scale
	FVector scale = portalTransform.GetScale3D();
	scale.Y *= -1; scale.Z *= -1;
	// MakeTransform
	portalTransform.SetScale3D(scale);

	// GetPlayerCameraManager
	APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
	if (!CameraManager)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("CameraManager not found"));

	// InverseTransformLocation
	FVector inversedLocation = portalTransform.InverseTransformPosition(CameraManager->GetCameraLocation());

	// Linked Portal & GetActorTransform
	FTransform otherPortalTransform = otherPortal->GetTransform();

	// TransformLocation
	FVector newLocation = otherPortalTransform.TransformPosition(inversedLocation);

	// SetWorldLocationAndRotation
	otherPortal->portalCamera->SetWorldLocationAndRotation(newLocation, FRotator::ZeroRotator);

}

