// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Portal.generated.h"

UCLASS()
class PUZZLEKIT_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APortal();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
private:
	void Setup() {
        // Create the plane mesh component
        frameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("frameMesh"));
        RootComponent = frameMesh;

        // Set the default static mesh for the frame mesh
        static ConstructorHelpers::FObjectFinder<UStaticMesh> FrameMeshAsset(TEXT("/Game/Portals/Portal_Frame"));
        if (FrameMeshAsset.Succeeded())
        {
            frameMesh->SetStaticMesh(FrameMeshAsset.Object);
        }

        // Create the frame mesh component and attach it to the plane mesh
        planeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("planeMesh"));
        planeMesh->SetupAttachment(frameMesh);

        // Set the default static mesh for the plane mesh
        static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshAsset(TEXT("/Game/Portals/Plane"));
        if (PlaneMeshAsset.Succeeded())
        {
            planeMesh->SetStaticMesh(PlaneMeshAsset.Object);
        }

        // Create an arrow component
        forwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("forwardDirection"));

        forwardDirection->SetupAttachment(frameMesh);

        // Get the forward vector of the frame mesh
        FVector ForwardVector = frameMesh->GetUpVector();

        // Set the rotation of the arrow component to match the forward vector of the frame mesh
        forwardDirection->SetWorldRotation(ForwardVector.Rotation());
	}
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* planeMesh;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* frameMesh;

	UPROPERTY(EditAnywhere)
	UArrowComponent* forwardDirection;

};
