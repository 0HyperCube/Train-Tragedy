// Fill out your copyright notice in the Description page of Project Settings.

#include "SegementConnector.h"
#include <Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h>
#include <Runtime/Engine/Public/DrawDebugHelpers.h>
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineMeshComponent.h"
#include "TrackSegement.h"


// Sets default values
ASegementConnector::ASegementConnector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	LocationComp = CreateDefaultSubobject<USceneComponent>(TEXT("LocationComp"));
	SetRootComponent(LocationComp);

	ChangePointsCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PointCollision"));
	ChangePointsCollision->AttachToComponent(LocationComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	ChangePointsCollision->SetCollisionProfileName("PointTrigger");
	ChangePointsCollision->SetBoxExtent(FVector(700, 500, 100));
	ChangePointsCollision->SetRelativeLocation(FVector(400, 0, 0));

	DirectionArrow = CreateDefaultSubobject<USplineMeshComponent>(TEXT("DirectionArrow"));

}

void ASegementConnector::OnConstruction(const FTransform& Transform)
{
	RemoveBadSplinePoints();
	PlaceEndsOfTracks();

	PlaceDirectionArrow();
}

// Called when the game starts or when spawned
void ASegementConnector::BeginPlay()
{
	Super::BeginPlay();

	RemoveBadSplinePoints();

	PlaceDirectionArrow();
	
}

// Called every frame
void ASegementConnector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LerpAlpha < 1) {
		LerpAlpha += DeltaTime/.1f;
		PlaceDirectionArrow();
	}

}

void ASegementConnector::ChangeDirection()
{
	PlaceDirectionArrow();

	OldInPosition	= InPosition;
	OldInTangent	= InTangent;
	OldOutPosition	= OutPosition;
	OldOutTangent	= OutTangent;

	LerpAlpha = 0.f;

	InTrackIndex += 1;
	InTrackIndex = InTrackIndex % InTracks.Num();

	OutTrackIndex += 1;
	OutTrackIndex = OutTrackIndex % OutTracks.Num();

	PlaceDirectionArrow();
}

void ASegementConnector::RemoveBadSplinePoints()
{
	for (int i = 0; i < OutTracks.Num(); i++) {
		ATrackSegement* track = OutTracks[i];
		if (track == nullptr || track->OutConnector != this) {
			OutTracks.RemoveAt(i);
			i -= 1;
		}
	}

	for (int i = 0; i < InTracks.Num(); i++) {
		ATrackSegement* track = InTracks[i];
		if (track == nullptr || track->InConnector != this) {
			InTracks.RemoveAt(i);
			i -= 1;
		}
	}

	

}

void ASegementConnector::PlaceEndsOfTracks()
{

	for (ATrackSegement* Track : InTracks) {
		Track->PlaceEnds();
	}

	for (ATrackSegement* Track : OutTracks) {
		Track->PlaceEnds();
	}
}

void ASegementConnector::PlaceDirectionArrow()
{

	if (InTracks.Num() < 1 || OutTracks.Num() < 1) {
		return;
	}

	float offset = InTracks.Num() > 1 ? 1500.f : 0.f; 

	InPosition	= GetInTrack()->SplineComponent->GetLocationAtDistanceAlongSpline	(offset, ESplineCoordinateSpace::World);
	InTangent	= GetInTrack()->SplineComponent->GetDirectionAtDistanceAlongSpline	(offset, ESplineCoordinateSpace::World)*1500;

	offset = InTracks.Num() > 1 ? 0.f : 1500.f;

	float length = GetOutTrack()->SplineComponent->GetSplineLength();
	OutPosition = GetOutTrack()->SplineComponent->GetLocationAtDistanceAlongSpline	(length-offset, ESplineCoordinateSpace::World);
	OutTangent	= GetOutTrack()->SplineComponent->GetDirectionAtDistanceAlongSpline	(length-offset, ESplineCoordinateSpace::World)*1500;


	DirectionArrow->SetWorldLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);

	if(InTracks.Num()>1)
		DirectionArrow->SetStartAndEnd(
			FMath::Lerp(OldInPosition,	InPosition,		LerpAlpha),
			FMath::Lerp(-OldInTangent,	-InTangent,		LerpAlpha),
			FMath::Lerp(OldOutPosition,	OutPosition,	LerpAlpha),
			FMath::Lerp(-OldOutTangent,	-OutTangent,	LerpAlpha)
		);
	else
		DirectionArrow->SetStartAndEnd(
			FMath::Lerp(OldOutPosition, OutPosition,	LerpAlpha),
			FMath::Lerp(OldOutTangent,	OutTangent,		LerpAlpha),
			FMath::Lerp(OldInPosition,	InPosition,		LerpAlpha),
			FMath::Lerp(OldInTangent,	InTangent,		LerpAlpha)
			
		);

	
}


float ASegementConnector::FindNewDirection(float currentDirection, ATrackSegement* currentTrack)
{
	bool isGoingIn = InTracks.Contains(currentTrack);
	bool isPositive = currentDirection > 0;

	if ((isGoingIn && isPositive) || (!isGoingIn && !isPositive)) {
		return currentDirection * -1;
	}
	return currentDirection;
}
