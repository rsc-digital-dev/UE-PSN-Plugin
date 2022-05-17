// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PSNStream.h"
#include "Common/UdpSocketReceiver.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

/** Interface for internal networking implementation. */
class POSISTAGENET_API IPSNSenderProxy
{
public:
	virtual ~IPSNSenderProxy() {}
	virtual void GetSendIPAddress(FString& InIPAddress, int32& Port) const = 0;
	virtual bool SetSendIPAddress(const FString& InIPAddress, const int32 Port) = 0;
	virtual void SendPSNData(TArray<FPSNTracker> TrackerData, uint64 Lifetime) = 0;
	virtual void SendPSNInfo(TArray<FPSNTracker> TrackerData, uint64 Lifetime) = 0;
	virtual void Stop() = 0;
};

class POSISTAGENET_API FPSNSenderProxy : public IPSNSenderProxy
{
public:

	// Ctor
	FPSNSenderProxy(const FString& InClientName);

	// Get  IP Address
	void GetSendIPAddress(FString& InIPAddress, int32& Port) const override;

	// Set IP Address
	bool SetSendIPAddress(const FString& InIPAddress, const int32 Port) override;

	// Send PSN Data
	void SendPSNData(TArray<FPSNTracker> TrackerData, uint64 Lifetime);

	void SendPSNInfo(TArray<FPSNTracker> TrackerData, uint64 Lifetime);

	// Stop Socket. Handles itself on EndPlay so no need to call then.
	void Stop();

private:

	void SendPacket(strlist packet);

	FSocket* Socket;

	TSharedPtr<FInternetAddr> IPAddress;

	FString SenderName;

	// Encoder. Single encoder exists once per Proxy since we need data persistence for correct iterations of packets. 
	TUniquePtr<class ::psn::psn_encoder> psn_encoder;

};
