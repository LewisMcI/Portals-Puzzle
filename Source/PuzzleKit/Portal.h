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

        // Create the frame mesh component and attach it to the plane mesh
        planeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("planeMesh"));
        planeMesh->SetupAttachment(frameMesh);

        // Create an arrow component
        forwardDirection = CreateDefaultSubobject<UArrowComponent>(TEXT("forwardDirection"));

        forwardDirection->SetupAttachment(frameMesh);

        // Get the forward vector of the frame mesh
        FVector ForwardVector = frameMesh->GetUpVector();

        // Set the rotation of the arrow component to match the forward vector of the frame mesh
        forwardDirection->SetWorldRotation(ForwardVector.Rotation());

        portalCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("portalCamera"));
        portalCamera->SetupAttachment(planeMesh);;

        // Set the rotation of the arrow component to match the forward vector of the frame mesh
        portalCamera->SetWorldRotation(ForwardVector.Rotation());
	}
    void BeginVisuals() {

		// Create New Mat Instance
		portalMatInstance = UMaterialInstanceDynamic::Create(portalMat, nullptr);
		if (!portalMatInstance) {
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("No Portal Material Setup"));
			return;
		}
		planeMesh->SetMaterial(0, portalMatInstance);

		// Create new Render Target
		portalRT = CreateRenderTargetWithTruncatedSize();
		if (!portalRT) {
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("No Render Target Setup"));
			return;
		}

		// Setup Portal Mat
		portalMatInstance->SetTextureParameterValue("Texture", portalRT);

		otherPortal->portalCamera->TextureTarget = portalRT;

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("All Working"));

		SetClipPlanes();
    }
    void UpdateSceneCapture();

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
    // Inside a function or class method where you want to create the render target with truncated size
    UTextureRenderTarget2D* CreateRenderTargetWithTruncatedSize()
    {
        FVector2D TruncatedViewportSize = GetTruncatedViewportSize();

        // Create a new render target
        UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();

        if (!RenderTarget)
        {
            // Handle the case where the render target creation failed
            UE_LOG(LogTemp, Error, TEXT("Failed to create render target."));
            return nullptr;
        }

        // Initialize the render target with truncated dimensions
        RenderTarget->InitCustomFormat(TruncatedViewportSize.X, TruncatedViewportSize.Y, PF_B8G8R8A8, false);
        RenderTarget->ClearColor = FLinearColor::Black;

        return RenderTarget;
    }
    // Function to get the viewport size
    FVector2D GetTruncatedViewportSize()
    {
        FVector2D ViewportSize(ForceInit);

        // Get the GameViewportClient
        UGameViewportClient* ViewportClient = GEngine->GameViewport;

        if (ViewportClient)
        {
            // Get the size of the viewport
            ViewportSize = ViewportClient->Viewport->GetSizeXY();

            // Truncate the dimensions as needed (e.g., cast to int32 to discard decimal parts)
            int32 TruncatedWidth = FMath::TruncToInt(ViewportSize.X);
            int32 TruncatedHeight = FMath::TruncToInt(ViewportSize.Y);

            ViewportSize.Set(TruncatedWidth, TruncatedHeight);
        }
        else
        {
            // Handle the case where the viewport client is not available
            UE_LOG(LogTemp, Warning, TEXT("GameViewportClient is not available."));
        }

        return ViewportSize;
    }
    // Setup Clip Planes
    void SetClipPlanes() {
        if (otherPortal->portalCamera == nullptr)
            return;

        // Enable clip plane
        otherPortal->portalCamera->bEnableClipPlane = true;

        // Set clip plane base
        FVector portalLoc = planeMesh->GetComponentLocation();
        FVector forwardVec= forwardDirection->GetForwardVector();

        otherPortal->portalCamera->ClipPlaneBase = (portalLoc + (forwardVec*-3.0f));

        // Set clip plane normal
        otherPortal->portalCamera->ClipPlaneNormal = forwardVec;
    }

    FVector SceneCaptureUpdateLocation() {
        // GetActorTransform
        FTransform portalTransform = GetActorTransform();
        // Set Scale
        FVector scale = portalTransform.GetScale3D();
        scale.X *= -1; scale.Y *= -1;
        // MakeTransform
        portalTransform.SetScale3D(scale);

        // GetPlayerCameraManager
        APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
        if (!CameraManager)
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("CameraManager not found"));

        // InverseTransformLocation
        FVector inversedLocation = portalTransform.InverseTransformPosition(CameraManager->GetCameraLocation());

        // Linked Portal & GetActorTransform & TransformLocation
        return otherPortal->GetTransform().TransformPosition(inversedLocation);
    }
    FRotator SceneCaptureUpdateRotation() {
        // GetPlayerCameraManager
        APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0));
        if (!CameraManager)
            GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("CameraManager not found"));

        // GetActorTransform
        FTransform portalTransform = GetActorTransform();
        FVector cameraDirection = CameraManager->GetCameraRotation().Vector();
        // X-Axis
        FVector portalDirectionX = portalTransform.InverseTransformVector(cameraDirection.XAxisVector);
        portalDirectionX = portalDirectionX.MirrorByVector(FVector(1.0, 0.0, 0.0));
        portalDirectionX = portalDirectionX.MirrorByVector(FVector(0.0, 1.0, 0.0));
        
        FVector forward = otherPortal->GetActorTransform().TransformVector(portalDirectionX);

        // Y-Axis
        FVector portalDirectionY = portalTransform.InverseTransformVector(cameraDirection.YAxisVector);
        portalDirectionY = portalDirectionY.MirrorByVector(FVector(1.0, 0.0, 0.0));
        portalDirectionY = portalDirectionY.MirrorByVector(FVector(0.0, 1.0, 0.0));

        FVector right = otherPortal->GetActorTransform().TransformVector(portalDirectionY);

        // Z-Axis
        FVector portalDirectionZ = portalTransform.InverseTransformVector(cameraDirection.ZAxisVector);
        portalDirectionZ = portalDirectionZ.MirrorByVector(FVector(1.0, 0.0, 0.0));
        portalDirectionZ = portalDirectionZ.MirrorByVector(FVector(0.0, 1.0, 0.0));

        FVector up = otherPortal->GetActorTransform().TransformVector(portalDirectionZ);

        // Create a rotation matrix using the provided axes
        FMatrix RotMatrix(forward, right, up, FVector::ZeroVector);

        // Convert the rotation matrix to a rotator
        return RotMatrix.Rotator();        
    }
	UMaterialInstanceDynamic* portalMatInstance;
	UTextureRenderTarget2D* portalRT;
};
