// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PSNMessage.h"
#include "PSNSenderProxy.h"
#include "Tickable.h"
#include "PSNSenderSubsystem.generated.h"

/**
 *	PSN Sender Subsystem. 
 */
UCLASS()
class POSISTAGENET_API UPSNSenderSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:

	UPSNSenderSubsystem();

	//~ Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject Interface

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="PSN", meta=(AdvancedDisplay = "ForceReset"))
	void StartPSNSender(FString IPAddress = TEXT("236.10.10.10"), int32 Port = 56565, const FString& SystemName = "Unreal Engine", EPSNFrequency Frequency = EPSNFrequency::PSN_60Hz, bool ForceReset = false);

	UFUNCTION(BlueprintCallable, Category = "PSN")
	void AddTracker(FPSNTrackerInfo TrackerInfo);

	UFUNCTION(BlueprintCallable, Category = "PSN")
	void UpdateTracker(FName TrackerName, FPSNTrackerData NewData);

	// Tracker Component is a live updated component
	UFUNCTION(BlueprintCallable, Category = "PSN")
	void AddComponentToTrack(FPSNTrackerInfo Meta, USceneComponent* Component);

	uint64 GetTimestamp();

	UFUNCTION()
	void SendData();

	UFUNCTION()
	void SendInfo();

	UFUNCTION(BlueprintPure, Category = "PSN")
	bool IsSenderRunning();

	UFUNCTION(BlueprintPure, Category = "PSN", meta=(StartFrom=1))
	int FindFreeID(int StartFrom);

protected:

	// Timer Handle for the Sender
	FTimerHandle TimerDataPacket;
	FTimerHandle TimerInfoPacket;

private:

	// User Chosen System Name
	FString ChosenSystemName;

	bool GetLocalHostAddress(FString& Address);

	void BuildTrackerList();

	int CheckAvailableID(int InID);

	TUniquePtr<IPSNSenderProxy> SenderPtr;

	EPSNFrequency ChosenFrequency;
	
	// Map to store all component ptrs that we wish to send
	TMap<FPSNTrackerInfo, USceneComponent*> ComponentMap;
	
	// Map to store all manually added trackers we wish to send
	TMap<FName, FPSNTracker> TrackerMap;

	// The combined tracker list, generated when using SendData, and cached so it can be send in the info packet.
	TArray<FPSNTracker> TrackerList;

};
