// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.


#include "PSNReceiverSubsystem.h"
#include "PSNReceiverProxy.h"

UPSNReceiverSubsystem::UPSNReceiverSubsystem()
	: ReceiverProxy(nullptr)
{
}

void UPSNReceiverSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPSNReceiverSubsystem::Deinitialize()
{
	Super::Deinitialize();
	StopReceiver();
}

void UPSNReceiverSubsystem::StartPSNReceiver(FString ReceiverName, FString IPAddress, int32 Port, bool bMulticastLoopback, bool bStartListening)
{
	if (ReceiverProxy)
	{
		UE_LOG(LogPSN, Warning, TEXT("PSN Receiver start was requested, but a PSN receiver is already running. Ignoring Request"));
		return;
	}

	ReceiverProxy.Reset(new FPSNReceiverProxy(*this));
	ReceiverProxy->SetMulticastLoopback(bMulticastLoopback);
	ReceiverProxy->SetAddress(IPAddress, Port);
	if (bStartListening)
	{
		ReceiverProxy->Listen(ReceiverName);
	}
}

void UPSNReceiverSubsystem::StopReceiver()
{
	if (ReceiverProxy.IsValid())
	{
		ReceiverProxy->Stop();
	}
}

// Called from the ReceiverProxy
void UPSNReceiverSubsystem::EnqueuePacket(TSharedPtr<FPSNTracker> Packet)
{
	PacketQueue.Enqueue(Packet);
}

// Called from the ReceiverProxy
void UPSNReceiverSubsystem::OnPacketReceived(const FString& IPAddress)
{
	TSharedPtr<FPSNTracker> Msg;

	while (PacketQueue.Dequeue(Msg))
	{
		DispatchMessage(Msg);
	}
}

void UPSNReceiverSubsystem::DispatchMessage(TSharedPtr<FPSNTracker> Message)
{
	// Before sending, convert from meters to cm
	Message->Data.Position *= 100;
	Message->Data.TargetPosition *= 100;
	OnPSNPacketReceived.Broadcast(*Message);
	
	if (ReceiverProxy->GetLastPacketType() == EPSNPacketType::PSNType_Info)
	{
		OnPSNInfoPacketReceived.Broadcast(Message->Info);
	}
	else if (ReceiverProxy->GetLastPacketType() == EPSNPacketType::PSNType_Data)
	{
		OnPSNDataPacketReceived.Broadcast(*Message);
	}
}