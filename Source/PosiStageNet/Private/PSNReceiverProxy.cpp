// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.


#include "PSNReceiverProxy.h"
#include "PosiStageNet.h"
#include "PSNStream.h"
#include "Async/TaskGraphInterfaces.h"
#include "Common/UdpSocketBuilder.h"
#include "Sockets.h"
#include "Stats/Stats2.h"
#include "PSN/psn_lib.hpp"

FPSNReceiverProxy::FPSNReceiverProxy(UPSNReceiverSubsystem& InReceiver)
	: ReceiverSubsystem(&InReceiver)
	, Socket(nullptr)
	, SocketReceiver(nullptr)
	, Port(::psn::DEFAULT_UDP_PORT)
	, bMulticastLoopback(false)
{
	psn_decoder = new ::psn::psn_decoder;
}

bool FPSNReceiverProxy::GetMulticastLoopback() const
{
	return bMulticastLoopback;
}

bool FPSNReceiverProxy::IsActive() const
{
	return SocketReceiver != nullptr;
}

void FPSNReceiverProxy::Listen(const FString& ServerName)
{
	if (IsActive())
	{
		return;
	}

	FUdpSocketBuilder Builder(*ServerName);
	Builder.BoundToPort(Port);
	if (ReceiveIPAddress.IsMulticastAddress())
	{
		Builder.JoinedToGroup(ReceiveIPAddress);
		if (bMulticastLoopback)
		{
			Builder.WithMulticastLoopback();
		}
	}
	else
	{
		if (bMulticastLoopback)
		{
			UE_LOG(LogPSN, Warning, TEXT("PSN Receiver '%s' ReceiveIPAddress provided is not a multicast address."),*ServerName);
		}
		Builder.BoundToAddress(ReceiveIPAddress);
	}

	Socket = Builder.Build();
	if (Socket)
	{
		SocketReceiver = new FUdpSocketReceiver(Socket, FTimespan::FromMilliseconds(100), *(ServerName + TEXT("_ListenerThread")));
		SocketReceiver->OnDataReceived().BindRaw(this, &FPSNReceiverProxy::OnPacketReceived);
		SocketReceiver->Start();
		UE_LOG(LogPSN, Display, TEXT("PSNReceiver '%s' Listening: %s:%d."), *ServerName, *ReceiveIPAddress.ToString(), Port);
	}
	else
	{
		UE_LOG(LogPSN, Error, TEXT("PSNReceiver '%s' failed to bind to socket on %s:%d."), *ServerName, *ReceiveIPAddress.ToString(), Port);
		ESocketErrors Error = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLastErrorCode();
		FString ErrorMsg = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetSocketError();
		UE_LOG(LogPSN, Error, TEXT("Potential Error: %s."), *ErrorMsg);
	}
}


bool FPSNReceiverProxy::SetAddress(const FString& InReceiveIPAddress, int32 InPort)
{
	if (IsActive())
	{
		UE_LOG(LogPSN, Error, TEXT("Cannot set address while PSNServer is active."));
		return false;
	}

	if (!FIPv4Address::Parse(InReceiveIPAddress, ReceiveIPAddress))
	{
		UE_LOG(LogPSN, Error, TEXT("Invalid ReceiveIPAddress '%s'. OSCServer ReceiveIP Address not updated."), *InReceiveIPAddress);
		return false;
	}

	Port = InPort;
	return true;
}

void FPSNReceiverProxy::SetMulticastLoopback(bool InMulticastLoopback)
{
	if (InMulticastLoopback != bMulticastLoopback && IsActive())
	{
		UE_LOG(LogPSN, Error, TEXT("Cannot update MulticastLoopback while OSCServer is active."));
		return;
	}

	bMulticastLoopback = InMulticastLoopback;
}

void FPSNReceiverProxy::Stop()
{
	if (SocketReceiver)
	{
		delete SocketReceiver;
		SocketReceiver = nullptr;
	}
	if (Socket)
	{
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

void FPSNReceiverProxy::OnPacketReceived(const FArrayReaderPtr& RawData, const FIPv4Endpoint& Endpoint)
{
	// Create PSN Stream
	FPSNStream Stream = FPSNStream(RawData->GetData(), RawData->Num(), LastFrameID);
	
	TArray<FPSNTracker> TrackerMap;
	Stream.DecodeToTrackers(TrackerMap, psn_decoder);
	LastPacketType = Stream.StreamDataType;
	LastFrameID = Stream.GetHeaderFrameID();

	for (auto& Tracker : TrackerMap)
	{
		TSharedPtr<FPSNTracker> TrackerPtr = MakeShared<FPSNTracker>(Tracker);
		ReceiverSubsystem->EnqueuePacket(TrackerPtr);
	}

	// Dispatch task to  dequeue and processes each event (approaching it this way avoids problems with multiple executions per tick)
	DECLARE_CYCLE_STAT(TEXT("PSNReceiver.OnPacketReceived"), STAT_PSNReceiverOnPacketReceived, STATGROUP_PSNNetworkCommands);
	FFunctionGraphTask::CreateAndDispatchWhenReady([this, Endpoint]()
		{
		ReceiverSubsystem->OnPacketReceived(Endpoint.Address.ToString());
		}, GET_STATID(STAT_PSNReceiverOnPacketReceived), nullptr, ENamedThreads::GameThread);

}
