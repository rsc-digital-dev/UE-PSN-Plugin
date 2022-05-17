// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.

#pragma once

#include "PSNReceiverSubsystem.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Common/UdpSocketReceiver.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

class UPSNReceiverSubsystem;

/** Interface for internal networking implementation. */
class POSISTAGENET_API IPSNServerProxy
{
public:
	virtual ~IPSNServerProxy() { }
	virtual bool GetMulticastLoopback() const = 0;
	virtual bool IsActive() const = 0;
	virtual void Listen(const FString& ServerName) = 0;
	virtual bool SetAddress(const FString& InReceiveIPAddress, int32 InPort) = 0;
	virtual void SetMulticastLoopback(bool bInMulticastLoopback) = 0;
	virtual void Stop() = 0;
	virtual EPSNPacketType GetLastPacketType() = 0;
};


class POSISTAGENET_API FPSNReceiverProxy : public IPSNServerProxy
{
public:

	// Ctor
	FPSNReceiverProxy(UPSNReceiverSubsystem& InReceiverSubsystem);

	// Dtor
	virtual ~FPSNReceiverProxy() {}

	bool GetMulticastLoopback() const override;

	bool IsActive() const override;

	void Listen(const FString& ServerName) override;

	bool SetAddress(const FString& InReceiveIPAddress, int32 InPort) override;

	void SetMulticastLoopback(bool InMulticastLoopback) override;

	void Stop() override;

	/** Callback that receives data from a socket. */
	void OnPacketReceived(const FArrayReaderPtr& RawData, const FIPv4Endpoint& Endpoint);

	EPSNPacketType GetLastPacketType() override { return LastPacketType; }
	
private:

	/** Receiver Object */
	UPSNReceiverSubsystem* ReceiverSubsystem;
	
	/** Socket used to listen for PSN packets. */
	FSocket* Socket;
	
	/** UDP receiver. */
	FUdpSocketReceiver* SocketReceiver;

	/** IPAddress to listen for PSN packets on.  If unset, defaults to LocalHost */
	FIPv4Address ReceiveIPAddress;

	/** Port to listen for PSN packets on. */
	int32 Port;

	/** Whether or not to loopback if address provided is multicast */
	bool bMulticastLoopback;

	uint8_t LastFrameID;

	EPSNPacketType LastPacketType;

	::psn::psn_decoder* psn_decoder;

};
