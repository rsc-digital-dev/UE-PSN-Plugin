// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.

#pragma once

#include "PosiStageNet.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PSNMessage.h"
#include "PSNReceiverProxy.h"
//#include "UObject/Object.h"
#include "Async/TaskGraphInterfaces.h"
#include "Containers/Queue.h"
#include "Templates/UniquePtr.h"
//#include "Tickable.h" // may not be needed
#include "PSNReceiverSubsystem.generated.h"

class FSocket;
struct FTracker; // depreciate later
class IPSNServerProxy;

// On Packet Received Delegate. Catch-All for both Info and Data packets
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSNPacketReceivedEvent, const FPSNTracker&, Message);

// On Info Packet Received.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSNInfoPacketReceivedEvent, const FPSNTrackerInfo&, Message);

// On Data Packet Received.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPSNDataPacketReceivedEvent, const FPSNTracker&, Message);

// Stat Group, Mainly for Packet Queuing.
DECLARE_STATS_GROUP(TEXT("PSN Commands"), STATGROUP_PSNNetworkCommands, STATCAT_Advanced);

/**
 * PSN Receiver Subsystem
 */
UCLASS()
class POSISTAGENET_API UPSNReceiverSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UPSNReceiverSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "PSN")
	void StartPSNReceiver(FString ReceiverName = TEXT("Unreal Engine"), FString IPAddress = TEXT("236.10.10.10"), int32 Port = 56565, bool bMulticastLoopback = true, bool bStartListening = true);

	/** Force stop, closes ports. Called automatically on shutdown */
	UFUNCTION(BlueprintCallable, Category = "Posi Stage Net")
	void StopReceiver();

	/** Event OnPacketReceived. Catch-All for both data and info packets */
	UPROPERTY(BlueprintAssignable, Category = "Posi Stage Net")
	FPSNPacketReceivedEvent OnPSNPacketReceived;

	/** Event OnInfoPacketReceived */
	UPROPERTY(BlueprintAssignable, Category = "Posi Stage Net")
	FPSNInfoPacketReceivedEvent OnPSNInfoPacketReceived;

	/** Event OnDataPacketReceived */
	UPROPERTY(BlueprintAssignable, Category = "Posi Stage Net")
	FPSNDataPacketReceivedEvent OnPSNDataPacketReceived;

	/** Add Packet To Queue */
	void EnqueuePacket(TSharedPtr<FPSNTracker> Packet);

	/** On Packet Received, Add to Queue */
	void OnPacketReceived(const FString& IPAddress);

	/** Dispatch each message and fire delegate */
	void DispatchMessage(TSharedPtr<FPSNTracker> Message);

private:

	TUniquePtr<IPSNServerProxy> ReceiverProxy;

	// Queue
	TQueue<TSharedPtr<FPSNTracker>> PacketQueue;
};
