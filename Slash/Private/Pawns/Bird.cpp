// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawns/Bird.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"


// Sets default values
ABird::ABird() {
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Creating capsule for pawn
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetCapsuleHalfHeight(20.f);
	Capsule->SetCapsuleRadius(20.f);
	SetRootComponent(Capsule);

	// Setting up skeletal mesh for pawn
	BirdMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BirdMesh"));
	BirdMesh->SetupAttachment(GetRootComponent());

	// Adding spring arm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	// Adding camera
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	// Enabling auto possession for the pawn
	AutoPossessPlayer = EAutoReceiveInput::Player0;


}

// Called when the game starts or when spawned
void ABird::BeginPlay() {
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController())) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(BirdMappingContext, 0);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
/////
/////	Enhanced Input Movement Functions
/////
////////////////////////////////////////////////////////////////////////////////
void ABird::Move(const FInputActionValue& Value) {
	const float DirectionValue = Value.Get<float>();

	if (GetController() && (DirectionValue != 0.f)) {
		FVector Forward = GetActorForwardVector();
		AddMovementInput(Forward, DirectionValue);
	}
}

void ABird::Look(const FInputActionValue& Value) {
	const FVector2D LookAxisValue = Value.Get<FVector2D>();

	if (GetController()){
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

////////////////////////////////////////////////////////////////////////////////
/////
/////	Old Movement Input Functions
/////
////////////////////////////////////////////////////////////////////////////////
void ABird::MoveForward(float Value) {
	if (GetController() && (Value != 0.f)) {
		FVector Forward = GetActorForwardVector();
		AddMovementInput(Forward, Value);
	}
}

void ABird::Turn(float Value) {
	if (GetController() && (Value != 0.f)) {
		AddControllerYawInput(Value);
	}
}

void ABird::LookUp(float Value) {
	if (GetController() && (Value != 0.f)) {
		AddControllerPitchInput(Value);
	}
}
////////////////////////////////////////////////////////////////////////////////

// Called every frame
void ABird::Tick(float DeltaTime){
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABird::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Old axis mappings
	//PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ABird::MoveForward);
	//PlayerInputComponent->BindAxis(FName("Turn"), this, &ABird::Turn);
	//PlayerInputComponent->BindAxis(FName("LookUp"), this, &ABird::LookUp);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABird::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABird::Look);
	}
}

