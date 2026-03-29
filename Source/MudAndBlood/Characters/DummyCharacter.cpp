// Fill out your copyright notice in the Description page of Project Settings.


#include "DummyCharacter.h"

// Sets default values
ADummyCharacter::ADummyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADummyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADummyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADummyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

