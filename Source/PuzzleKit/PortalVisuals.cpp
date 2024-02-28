// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalVisuals.h"

PortalVisuals::PortalVisuals(UStaticMeshComponent* planeMesh)
{
	portalPlane = planeMesh;
}
void PortalVisuals::BeginPlay(UMaterial* portalMaterial, USceneCaptureComponent2D* otherPortalCameraRef) {
	otherPortalCamera = otherPortalCameraRef;

	// Create New Mat Instance
	portalMat = CreateDynamicMaterialInstance(portalMaterial);
	if (!portalMat){
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("No Portal Material Setup"));
		return;
	}
	portalPlane->SetMaterial(0, portalMat);
	
	// Create new Render Target
	portalRT = CreateRenderTargetWithTruncatedSize();
	if (!portalRT){
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("No Render Target Setup"));
		return;
	}

	// Setup Portal Mat
	portalMat->SetTextureParameterValue("Texture", portalRT);

	otherPortalCamera->TextureTarget = portalRT;

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("All Working"));
}

PortalVisuals::~PortalVisuals()
{
}
