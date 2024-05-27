// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

void ASlashHUD::PostInitializeComponents() {
	Super::PostInitializeComponents();

	UWorld* World = GetWorld();
	if (World) {
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller && SlashOverlayClass) {
			SlashOverlay = CreateWidget<USlashOverlay>(Controller, SlashOverlayClass);
			UE_LOG(LogTemp, Warning, TEXT("SlashOverlay set"));
			SlashOverlay->AddToViewport();
		}
	}
}
