// Fill out your copyright notice in the Description page of Project Settings.


#include "Portal.h"

// Sets default values
APortal::APortal()
{
 	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

    // Create a cube mesh component
    planeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("planeMesh"));
    RootComponent = planeMesh;

}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

