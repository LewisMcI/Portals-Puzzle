// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include <Components/SceneCaptureComponent2D.h>

/**
 * 
 */
class PUZZLEKIT_API PortalVisuals
{
public:
	PortalVisuals(UStaticMeshComponent* planeMesh);
	~PortalVisuals();

    void BeginPlay(UMaterial* portalMaterial, USceneCaptureComponent2D* otherPortalCameraRef);

    // Assuming you have a reference to the parent material `portalMat`
    UMaterialInstanceDynamic* CreateDynamicMaterialInstance(UMaterialInterface* ParentMaterial)
    {
        if (!ParentMaterial)
        {
            UE_LOG(LogTemp, Error, TEXT("Parent material is null."));
            return nullptr;
        }

        // Cast the parent material to UMaterialInterface if necessary
        UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(ParentMaterial, nullptr);

        if (!DynamicMaterialInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create dynamic material instance."));
            return nullptr;
        }

        // Optionally, you can set any parameter values for the dynamic material instance here
        // Example:
        // DynamicMaterialInstance->SetVectorParameterValue("MyParameterName", FVector(1.0f, 0.0f, 0.0f));

        return DynamicMaterialInstance;
    }
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

    UStaticMeshComponent* portalPlane;

    UMaterialInstanceDynamic* portalMat;

    UTextureRenderTarget2D* portalRT;

    USceneCaptureComponent2D* otherPortalCamera;
};
