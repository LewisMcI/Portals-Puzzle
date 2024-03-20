// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include "Kismet/KismetRenderingLibrary.h"
#include <Kismet/KismetMaterialLibrary.h>
#include "Blueprint/WidgetLayoutLibrary.h"

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
        root = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

        // Create the plane mesh component
        frameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("frameMesh"));

        // Create the frame mesh component and attach it to the plane mesh
        planeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("planeMesh"));

        // Create an arrow component
        forwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("forwardDirection"));

        // Get the forward vector of the frame mesh
        FVector ForwardVector = frameMesh->GetUpVector();

        // Set the rotation of the arrow component to match the forward vector of the frame mesh
        forwardDirection->SetWorldRotation(ForwardVector.Rotation());

        portalCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("portalCamera"));
        
        RootComponent = root;
        frameMesh->SetupAttachment(root);
        forwardDirection->SetupAttachment(root);
        planeMesh->SetupAttachment(root);
        portalCamera->SetupAttachment(root);

        // Set the rotation of the arrow component to match the forward vector of the frame mesh
        portalCamera->SetWorldRotation(ForwardVector.Rotation());
	}
    void BeginVisuals() {

		// Create New Mat Instance
        
		portalMatInstance = UKismetMaterialLibrary::CreateDynamicMaterialInstance(GetWorld(), portalMat);

		planeMesh->SetMaterial(0, portalMatInstance);

        // Create render target
        FVector2D viewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());

        int32 truncX = UKismetMathLibrary::FTrunc(viewportSize.X);
        int32 truncY = UKismetMathLibrary::FTrunc(viewportSize.Y);

        portalRT = UKismetRenderingLibrary::CreateRenderTarget2D(GetWorld(), truncX, truncY);

        // Setup Portal Mat
        portalMatInstance->SetTextureParameterValue("Texture", portalRT);

        otherPortal->portalCamera->TextureTarget = portalRT;
        
        SetClipPlanes();
    }
    void UpdateSceneCapture();

    UPROPERTY(EditAnywhere)
    USceneComponent* root;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* planeMesh;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* frameMesh;

	UPROPERTY(EditAnywhere)
	UArrowComponent* forwardDirection;

    UPROPERTY(EditAnywhere)
    UMaterial* portalMat;

    UPROPERTY(EditAnywhere)
    APortal* otherPortal;

    UPROPERTY(EditAnywhere)
    USceneCaptureComponent2D* portalCamera;

	// Visuals

    // Setup Clip Planes
    void SetClipPlanes() {
        if (otherPortal->portalCamera == nullptr)
            return;

        // Enable clip plane
        otherPortal->portalCamera->bEnableClipPlane = true;

        // Set clip plane base
        FVector portalLoc = planeMesh->GetComponentLocation();
        FVector forwardVec = forwardDirection->GetForwardVector();

        otherPortal->portalCamera->ClipPlaneBase = (portalLoc + (forwardVec*-3.0f));

        // Set clip plane normal
        otherPortal->portalCamera->ClipPlaneNormal = forwardVec;
    }
    void CheckViewportSize() {
        // Create render target
        FVector2D viewportSize = UWidgetLayoutLibrary::GetViewportSize(GetWorld());

        float x; float y;
        UKismetMathLibrary::BreakVector2D(viewportSize, x, y);

        if ((x == portalRT->SizeX) && (y == portalRT->SizeY)) {
        }
        else {
            UKismetSystemLibrary::PrintString(GetWorld(), "RESIZING");
            int32 truncX = UKismetMathLibrary::FTrunc(viewportSize.X);
            int32 truncY = UKismetMathLibrary::FTrunc(viewportSize.Y);
            UKismetRenderingLibrary::ResizeRenderTarget2D(portalRT, truncX, truncY);
        }
    }

    FVector SceneCaptureUpdateLocation() {
        // Set Scale
        FVector location; FRotator rotation; FVector scale;
        UKismetMathLibrary::BreakTransform(GetActorTransform(), location, rotation, scale);

        float x; float y; float z;
        UKismetMathLibrary::BreakVector(scale, x, y, z);
        FVector newScale = UKismetMathLibrary::MakeVector(x*-1, y*-1, z);

        FTransform newTransform = UKismetMathLibrary::MakeTransform(location, rotation, newScale);

        APlayerCameraManager* camMan = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
        USceneComponent* camManTransform = camMan->GetTransformComponent();
        FVector inverseLocation = UKismetMathLibrary::InverseTransformLocation(newTransform, camManTransform->GetComponentLocation());

        return UKismetMathLibrary::TransformLocation(otherPortal->GetActorTransform(), inverseLocation);
    }
    FRotator SceneCaptureUpdateRotation() {
        // Get Camera Manager
        APlayerCameraManager* camMan = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
        // Get transform of player camera manager
        USceneComponent* camManTransform = camMan->GetTransformComponent();
        FRotator camRotation = camManTransform->GetComponentRotation();
        
        // Break Rot into Axes
        FVector x = FVector::ZeroVector; FVector y = FVector::ZeroVector; FVector z = FVector::ZeroVector;
        UKismetMathLibrary::BreakRotIntoAxes(camRotation, x, y, z);

        FTransform portalTransform = GetActorTransform();
        FTransform otherPortalTransform = otherPortal->GetActorTransform();

        // Forward
        FVector inverseForward = UKismetMathLibrary::InverseTransformDirection(portalTransform, x);
        FVector mirrorForwardX = UKismetMathLibrary::MirrorVectorByNormal(inverseForward, FVector(1.0f, 0.0f, 0.0f));
        FVector mirrorForwardY = UKismetMathLibrary::MirrorVectorByNormal(mirrorForwardX, FVector(0.0f, 1.0f, 0.0f));
        FVector forward = UKismetMathLibrary::TransformDirection(otherPortalTransform, mirrorForwardY);

        return UKismetMathLibrary::MakeRotationFromAxes(forward, FVector::ZeroVector, FVector::ZeroVector);
        
    }
	UMaterialInstanceDynamic* portalMatInstance;
	UTextureRenderTarget2D* portalRT;
};
