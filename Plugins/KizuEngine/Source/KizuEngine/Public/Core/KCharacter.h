// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "KCharacter.generated.h"


USTRUCT(BlueprintType)
struct FResource {
	GENERATED_USTRUCT_BODY()

public:
	/** The custom resource name */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	FString Name = "None";
	/** The custom resource max value */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	float MaxValue = 100.f;
	/** The custom resource current value */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	float CurrentValue = 100.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	bool bCanBeBelowZero = false;
};

USTRUCT(BlueprintType)
struct FCharacterData 
{
	GENERATED_USTRUCT_BODY()

public:

	/** The character max health. The current health can never exceed this amount */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	float MaxHealth = 100.f;
	/** The character current health. When reduced to 0, they are considered dead. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	float CurrentHealth = 100.f;
	/** The character max energy. The current energy can never exceed this amount */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	float MaxEnergy = 100.f;
	/** The character current energy. When reduced to 0, they have some limitations. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	float CurrentEnergy = 100.f;
	/** Custom resources array (Examples: Energy, Mana, Armors..) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data")
	TArray<FResource> Resources;
};

UCLASS()
class KIZUENGINE_API AKCharacter : public ACharacter
{
	GENERATED_BODY()
		
public:

	/** The character's Data, containing all kind of general stats that vary during the Gameplay.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, ReplicatedUsing = OnRep_CharacterData, Category = "Kizu|Character|Data")
	FCharacterData CharacterData;
	/** If the character plays the death montage on death */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Kizu|Character|Data|Death")
	bool bPlayDeathMontage = false;
	/** This death montage is played on the death of the character (Current health equals 0)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition="bPlayDeathMontage"), Category = "Kizu|Character|Data|Death")
	UAnimMontage* DeathMontage;

	// Sets default values for this character's properties
	AKCharacter();
	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	/**
	 * General Character Data functionalities
	 */

	/** Sets the character data during the initialization or to use when changing multiple variables.*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Kizu|Character|Data")
	void ServerSetCharacterData(const FCharacterData& inCharacterData);
	/** Sets the character current health.*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Kizu|Character|Data")
	void ServerSetCurrentHealth(const float& inValue);
	/** Sets the character current energy.*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Kizu|Character|Data")
	void ServerSetCurrentEnergy(const float& inValue);
	/** Sets the character current energy.*/
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Kizu|Character|Data")
	void ServerSetCurrentResource(const FString& ResourceName, const float& inValue);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kizu|Character|Data")
	bool GetResource(const FString ResourceName, FResource& ResultResource);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kizu|Character|Data")
	bool GetResourceIndex(const FString ResourceName, int& ResourceIndex);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kizu|Character|Data")
	bool GetResourceCurrentValue(const FString ResourceName, float& ResultValue);
	UFUNCTION(BlueprintCallable, Category = "Kizu|Character|Data")
	bool GainResource(const FString ResourceName, const float ValueToGain = 10);
	UFUNCTION(BlueprintCallable, Category = "Kizu|Character|Data")
	bool ConsumeResource(const FString ResourceName, const float ValueToConsume = 10);
	UFUNCTION(BlueprintImplementableEvent, Category = "Kizu|Character|Data")
	void OnResourceGain(const FString& ResourceName, const float& Value);
	virtual void OnResourceGain_Native(const FString& ResourceName, const float& Value);
	UFUNCTION(BlueprintImplementableEvent, Category = "Kizu|Character|Data")
	void OnResourceConsumption(const FString& ResourceName, const float& Value);
	virtual void OnResourceConsumption_Native(const FString& ResourceName, const float& Value);

	/**
	 * Character Combat functionalities
	 */

	/** Apply damage to an Actor (replicated).*/
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Kizu|Character|Damage")
	void ServerApplyDamage(AActor* Target, const float Damage, TSubclassOf<UDamageType> DamageType);
	
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


	/**
	 * Character Resources management
	 */

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CharacterData();
	/** Response to Data being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Kizu|Character|Data")
	void OnCurrentHealthChange();
	virtual void OnCurrentHealthChange_Native();

	/** Execute the death event (Animation/Events/etc...) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Kizu|Character|Data")
	void ExecuteDeathEvent();
	virtual void ExecuteDeathEvent_Native();
	



	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * Returns Checks if the player is networked
	 * @return True if the player is networked, false if it's a standalone object
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kizu|Character|Network")
	uint8 GetIsNetworked();

	/**
	 * Play Montage on server and all clients (Through the AnimInstance of the main Mesh)
	 * @param Montage The montage to be played by the AnimInstance
	 * @param Rate Play rate of the Montage
	 */
	UFUNCTION(BlueprintCallable, Category = "Kizu|Character|Animation")
	void MontagePlay_Replicated(UAnimMontage* Montage, const float Rate = 1.f);
	/**
	 * Play Montage on server and all clients (Through the AnimInstance of the main Mesh)
	 * @param Montage The montage to be played by the AnimInstance
	 * @param Rate Play rate of the Montage
	 */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Kizu|Character|Animation")
	void ServerMontagePlay(UAnimMontage* Montage, const float Rate = 1.f);
	/**
	 * Play Montage on client (Through the AnimInstance of the main Mesh)
	 * @param Montage The montage to be played by the AnimInstance
	 * @param Rate Play rate of the Montage
	 */
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Kizu|Character|Animation")
	void ClientMontagePlay(UAnimMontage* Montage, const float Rate = 1.f);
	/**
	 * Play Montage on all clients (Through the AnimInstance of the main Mesh)
	 * @param Montage The montage to be played by the AnimInstance
	 * @param Rate Play rate of the Montage
	 */
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category = "Kizu|Character|Animation")
	void MulticastMontagePlay(UAnimMontage* Montage, const float Rate = 1.f);



};
