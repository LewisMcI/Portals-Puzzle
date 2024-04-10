// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include "Kismet/KismetRenderingLibrary.h"
#include <Kismet/KismetMaterialLibrary.h>
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/Character.h" 
#include "GameFramework/PawnMovementComponent.h"

#include "Portal.generated.h"

UCLASS()
class PUZZLEKIT_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	APortal();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
    
    /* Portal Setup */
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

        objectDetection = CreateDefaultSubobject<UBoxComponent>(TEXT("objectDetection"));

        RootComponent = root;
        objectDetection->SetupAttachment(root);
        frameMesh->SetupAttachment(root);
        forwardDirection->SetupAttachment(root);
        planeMesh->SetupAttachment(root);
        portalCamera->SetupAttachment(root);
        
        // Set the rotation of the arrow component to match the forward vector of the frame mesh
        portalCamera->SetWorldRotation(ForwardVector.Rotation());
    }
    FVector InvertLocation(FVector lastLocation) {
        // Set Scale
        FVector location; FRotator rotation; FVector scale;
        UKismetMathLibrary::BreakTransform(GetActorTransform(), location, rotation, scale);

        float x; float y; float z;
        UKismetMathLibrary::BreakVector(scale, x, y, z);
        FVector newScale = UKismetMathLibrary::MakeVector(x * -1, y * -1, z);

        FTransform newTransform = UKismetMathLibrary::MakeTransform(location, rotation, newScale);

        FVector inverseLocation = UKismetMathLibrary::InverseTransformLocation(newTransform, lastLocation);

        return UKismetMathLibrary::TransformLocation(otherPortal->GetActorTransform(), inverseLocation);
    }
    FRotator InvertRotation(FRotator rotation) {
        // Break Rot into Axes
        FVector x = FVector::ZeroVector; FVector y = FVector::ZeroVector; FVector z = FVector::ZeroVector;
        UKismetMathLibrary::BreakRotIntoAxes(rotation, x, y, z);

        FTransform portalTransform = GetActorTransform();
        FTransform otherPortalTransform = otherPortal->GetActorTransform();

        // Forward
        FVector inverseForward = UKismetMathLibrary::InverseTransformDirection(portalTransform, x);
        FVector mirrorForwardX = UKismetMathLibrary::MirrorVectorByNormal(inverseForward, FVector(1.0f, 0.0f, 0.0f));
        FVector mirrorForwardY = UKismetMathLibrary::MirrorVectorByNormal(mirrorForwardX, FVector(0.0f, 1.0f, 0.0f));
        FVector forward = UKismetMathLibrary::TransformDirection(otherPortalTransform, mirrorForwardY);

        // Right
        FVector inverseRight = UKismetMathLibrary::InverseTransformDirection(portalTransform, y);
        FVector mirrorRightX = UKismetMathLibrary::MirrorVectorByNormal(inverseRight, FVector(1.0f, 0.0f, 0.0f));
        FVector mirrorRightY = UKismetMathLibrary::MirrorVectorByNormal(mirrorRightX, FVector(0.0f, 1.0f, 0.0f));
        FVector right = UKismetMathLibrary::TransformDirection(otherPortalTransform, mirrorRightY);

        // Up
        FVector inverseUp = UKismetMathLibrary::InverseTransformDirection(portalTransform, z);
        FVector mirrorUpX = UKismetMathLibrary::MirrorVectorByNormal(inverseUp, FVector(1.0f, 0.0f, 0.0f));
        FVector mirrorUpY = UKismetMathLibrary::MirrorVectorByNormal(mirrorUpX, FVector(0.0f, 1.0f, 0.0f));
        FVector up = UKismetMathLibrary::TransformDirection(otherPortalTransform, mirrorUpY);

        return UKismetMathLibrary::MakeRotationFromAxes(forward, right, up);
    }

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

    UPROPERTY(EditAnywhere)
    UBoxComponent* objectDetection;

	/* Visuals */
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

        FVector offset = forwardDirection->GetForwardVector() * offsetAmount;
        FLinearColor colour = UKismetMathLibrary::MakeColor(offset.X, offset.Y, offset.Z, 1.0f);

        portalMatInstance->SetVectorParameterValue("OffsetDistance", colour);
    }
    FVector SceneCaptureUpdateLocation() {
        return InvertLocation(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation());
    }
    FRotator SceneCaptureUpdateRotation() {
        return InvertRotation(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraRotation());
        
    }
    void UpdateSceneCapture();

	UMaterialInstanceDynamic* portalMatInstance;
	UTextureRenderTarget2D* portalRT;

    /* Teleportation*/
    void ShouldTeleport() {
        TArray<AActor*> OverlappingActors;
        
        objectDetection->GetOverlappingActors(OverlappingActors);

        for (AActor* Actor : OverlappingActors)
        {
            // For whatever reason I can't seem to include the BP_ThirdPersonCharacter.h, so this is my roundabout solution
            FString ActorClassName = Actor->GetClass()->GetName();
            
            // Player
            if (FCString::Strcmp(*ActorClassName, TEXT("BP_ThirdPersonCharacter_C")) == 0)
            {
                APlayerCameraManager* camMan = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
                FVector point = camMan->GetCameraLocation();
                FVector portalLocation = GetActorLocation();
                FVector portalNormal = forwardDirection->GetForwardVector();
                if (IsPointCrossingPortal(point, portalLocation, portalNormal))
                    TeleportCharacter();
            }
            if (FCString::Strcmp(*ActorClassName, TEXT("BP_Portal_C")) == 0)
                continue;
            UKismetSystemLibrary::PrintString(GetWorld(), ActorClassName);
        }
        // Other Objects
        // Get the world pointer
        UWorld* World = GetWorld();

        if (World)
        {
            // Get the bounding box of the box component in world space
            FBox BoxBounds = objectDetection->CalcBounds(objectDetection->GetComponentTransform()).GetBox();

            // Query parameters
            FCollisionQueryParams Params;
            Params.bTraceComplex = true; // Trace against complex collision (meshes)

            // Array to store hit results
            TArray<FHitResult> HitResults;

            // Perform the collision query
            World->SweepMultiByObjectType(HitResults, BoxBounds.GetCenter(), BoxBounds.GetCenter(), FQuat::Identity,
                FCollisionObjectQueryParams::AllObjects, objectDetection->GetCollisionShape(), Params);

            // Iterate over hit results
            for (const FHitResult& HitResult : HitResults)
            {
                AActor* HitActor = HitResult.GetActor();
                if (HitActor && HitActor != GetOwner()) // Exclude the owner actor if necessary
                {
                    // Do something with the overlapping actor
                    FString ActorClassName = HitActor->GetName();

                    if (ActorClassName == "BP_Item_CubeXL_C_0") {
                        UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Colliding with Interactable"));

                        TeleportObject(HitActor);
                    }
                }
            }

            // Debug visualization of the box component
            DrawDebugBox(World, BoxBounds.GetCenter(), BoxBounds.GetExtent(), FQuat::Identity, FColor::Red, false, -1, 0, 1);
        }
    }

    void TeleportObject(AActor* actor) {
        APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

        // Location
        FVector newLocation = InvertLocation(actor->GetActorLocation());

        // Rotation
        // Get Camera Manager
        FRotator newRotation = InvertRotation(actor->GetActorRotation());

        // Scale
        FVector newScale = FVector(1.0f);

        // New Actor Transform
        FTransform newTransform = UKismetMathLibrary::MakeTransform(newLocation, newRotation, newScale);
        actor->SetActorTransform(newTransform);

        // Control Rotation
        FRotator controlRotation = InvertRotation(playerController->GetControlRotation());;
        actor->SetActorRotation(controlRotation);

        // Handle Momentum
    /*    FVector velocity = actor->
        actor-> = UpdateVelocity(movement->Velocity);*/

        UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Teleport Object"));
    }

    void TeleportCharacter() {
        APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        ACharacter* playerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

        // Location
        FVector newLocation = InvertLocation(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetActorLocation());
        
        // Rotation
        // Get Camera Manager
        FRotator newRotation = InvertRotation(playerCharacter->GetActorRotation());

        // Scale
        FVector newScale = FVector(1.0f);

        // New Actor Transform
        FTransform newTransform = UKismetMathLibrary::MakeTransform(newLocation, newRotation, newScale);
        playerCharacter->SetActorTransform(newTransform);

        // Control Rotation
        FRotator controlRotation = InvertRotation(playerController->GetControlRotation());;
        playerController->SetControlRotation(controlRotation);
        
        // Handle Momentum
        UPawnMovementComponent* movement = playerCharacter->GetMovementComponent();
        movement->Velocity = UpdateVelocity(movement->Velocity);

        UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->SetGameCameraCutThisFrame();

        UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Teleport Character"));
    }

    FVector UpdateVelocity(FVector velocity) {
        FVector location = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), UKismetMathLibrary::Vector_Normal2D(velocity, 0.0001));
        FVector mirrorX = UKismetMathLibrary::MirrorVectorByNormal(location, FVector(1.0f, 0.0f, 0.0f));
        FVector mirrorY = UKismetMathLibrary::MirrorVectorByNormal(mirrorX, FVector(0.0f, 1.0f, 0.0f));

        FVector direction = UKismetMathLibrary::TransformDirection(otherPortal->GetActorTransform(), mirrorY);
        
        return direction * velocity.Length();
    }
    bool IsPointCrossingPortal(FVector point, FVector portalLocation, FVector portalNormal) {
        FVector distance = point - portalLocation;
        double angle = UKismetMathLibrary::Dot_VectorVector(portalNormal, distance);

        bool isInFront = angle >= 0;

        FPlane portalPlane = UKismetMathLibrary::MakePlaneFromPointAndNormal(portalLocation, portalNormal);
        float t; FVector intersection;
        bool isIntersect = UKismetMathLibrary::LinePlaneIntersection(lastPosition, point, portalPlane, t, intersection);
        
        bool isCrossing = isIntersect && !isInFront && lastInFront;
        lastInFront = isInFront;
        lastPosition = point;

        return isCrossing;
    }

    FVector lastPosition;
    bool lastInFront;
    float offsetAmount = -6.0f;

    // Recursion

    void SceneCaptureUpdateRecursive(FVector location, FRotator rotation) {
        FVector tempLocation; FRotator tempRotation;
        if (currentRecursion == 0) {
            USceneComponent* camTransform = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetTransformComponent();
            FVector camLocation = camTransform->GetComponentLocation();
            tempLocation = InvertLocation(camLocation);
            FRotator camRotation = camTransform->GetComponentRotation();
            tempRotation = InvertRotation(camRotation);

            currentRecursion++;

            SceneCaptureUpdateRecursive(tempLocation, tempRotation);

            otherPortal->portalCamera->SetWorldLocationAndRotation(tempLocation, tempRotation);
            otherPortal->portalCamera->CaptureScene();
            currentRecursion = 0;
        }
        else if (currentRecursion < maxRecursions) {
            tempLocation = InvertLocation(location);
            tempRotation = InvertRotation(rotation);
            
            currentRecursion++;

            SceneCaptureUpdateRecursive(tempLocation, tempRotation);
            
            otherPortal->portalCamera->SetWorldLocationAndRotation(tempLocation, tempRotation);
            otherPortal->portalCamera->CaptureScene();
        }
        else {
            otherPortal->portalCamera->SetWorldLocationAndRotation(InvertLocation(location), InvertRotation(rotation));
            planeMesh->SetVisibility(false);
            otherPortal->portalCamera->CaptureScene();
            planeMesh->SetVisibility(true);
        }
    }

    int currentRecursion;
    int maxRecursions = 1;
};
