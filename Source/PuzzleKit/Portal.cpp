// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

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
	BeginVisuals();
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSceneCapture();
}

void APortal::UpdateSceneCapture()
{
	FVector newLocation = SceneCaptureUpdateLocation();
	FRotator newRotation = SceneCaptureUpdateRotation();
	// SetWorldLocationAndRotation
	otherPortal->portalCamera->SetWorldLocationAndRotation(newLocation, newRotation);

}

