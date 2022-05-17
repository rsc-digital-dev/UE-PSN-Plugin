// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.


#include "PSNSenderProxy.h"
#include "PosiStageNet.h"
#include "PSNStream.h"
#include "Common/UdpSocketReceiver.h"
#include "Common/UdpSocketBuilder.h"


FPSNSenderProxy::FPSNSenderProxy(const FString& InClientName)
	: Socket(FUdpSocketBuilder(*InClientName).Build())
	, psn_encoder(MakeUnique<::psn::psn_encoder>(TCHAR_TO_ANSI(*InClientName)))
{
	SenderName = InClientName;
}

void FPSNSenderProxy::GetSendIPAddress(FString& InIPAddress, int32& Port) const
{
	const bool bAppendPort = false;
	InIPAddress = IPAddress->ToString(bAppendPort);
	Port = IPAddress->GetPort();
}

bool FPSNSenderProxy::SetSendIPAddress(const FString& InIPAddress, const int32 Port)
{
	IPAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool bIsValidAddress = true;
	IPAddress->SetIp(*InIPAddress, bIsValidAddress);
	IPAddress->SetPort(Port);

	if (bIsValidAddress)
	{
		UE_LOG(LogPSN, Verbose, TEXT("PSNClient '%s' SetSendIpAddress: %s:%d"), *SenderName, *InIPAddress, Port);
	}
	else
	{
		UE_LOG(LogPSN, Warning, TEXT("PSNClient '%s' SetSendIpAddress Failed for input: %s:%d"), *SenderName, *InIPAddress, Port);
	}
	return bIsValidAddress;
}

void FPSNSenderProxy::SendPSNData(TArray<FPSNTracker> TrackerData, uint64 Lifetime)
{
	ensure(psn_encoder);

	// Check socket
	if (!Socket)
	{
		UE_LOG(LogPSN, Error, TEXT("Socket has been closed, unable to send PSN data"));
		return;
	}

	// Create Stream
	FPSNStream Stream = FPSNStream(TrackerData.Num());

	// Create Packet Variables
	strlist data_packets, info_packets;
		
	// Encode data and info packets to PSN
	Stream.EncodeToPSN(TrackerData, psn_encoder.Get(), data_packets, info_packets, Lifetime, false);

	// Send Data
	SendPacket(data_packets);

}

void FPSNSenderProxy::SendPSNInfo(TArray<FPSNTracker> TrackerData, uint64 Lifetime)
{
	// Check socket
	if (!Socket)
	{
		UE_LOG(LogPSN, Error, TEXT("Socket has been closed, unable to send PSN data"));
		return;
	}

	// Create Stream
	FPSNStream Stream = FPSNStream(TrackerData.Num());

	// Create Packet Variables
	strlist data_packets, info_packets;

	// Encode data and info packets to PSN
	Stream.EncodeToPSN(TrackerData, psn_encoder.Get(), data_packets, info_packets, Lifetime, true);

	// Send Data
	SendPacket(info_packets);
}

void FPSNSenderProxy::Stop()
{
	if (Socket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
}

void FPSNSenderProxy::SendPacket(strlist packet)
{
	int32 BytesSent = 0;

	for (auto it = packet.begin(); it != packet.end(); ++it)
	{
		int Length = it->length();
		const uint8* msg_t = (const uint8*)it->c_str();

		const bool bSendInfo = Socket->SendTo(msg_t, Length, BytesSent, *IPAddress);

		if (!bSendInfo)
		{
			UE_LOG(LogPSN, Error, TEXT("Failed to send Packets"));
		}
	}
}
