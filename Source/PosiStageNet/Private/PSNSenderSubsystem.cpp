// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.


#include "PSNSenderSubsystem.h"
#include "PosiStageNet.h"
#include "psn/psn_defs.hpp"

#include "Sockets.h"
#include "IPAddress.h"
#include "SocketSubsystem.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h" // Scene Components for Tracker
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UPSNSenderSubsystem::UPSNSenderSubsystem()
	: SenderPtr(nullptr)
{
}

void UPSNSenderSubsystem::Tick(float DeltaTime)
{
	if (IsTickable())
	{
		SendData();
	}
}

bool UPSNSenderSubsystem::IsTickable() const
{
	return (TrackerMap.Num() > 0 || ComponentMap.Num() > 0) && SenderPtr && ChosenFrequency == EPSNFrequency::PSN_OnTick;
}

TStatId UPSNSenderSubsystem::GetStatId() const
{
	return Super::GetStatID();
}

void UPSNSenderSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPSNSenderSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UPSNSenderSubsystem::StartPSNSender(FString IPAddress /*= TEXT("236.10.10.10")*/, int32 Port /*= 56565*/, const FString& SystemName /*= "Unreal Engine"*/, EPSNFrequency Frequency /*= EPSNFrequency::PSN_60Hz*/, bool ForceReset /*= true*/)
{
	if (IsSenderRunning() && ForceReset == false)
	{
		return;
	}

	ChosenSystemName = SystemName;
	SenderPtr.Reset(new FPSNSenderProxy(SystemName));
	SenderPtr->SetSendIPAddress(IPAddress, Port);
	ChosenFrequency = Frequency;
	
	check(GetWorld());
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	// Set up timers for a Hz send. For Tick, this is skipped and the OnTick will handle it.
	if (Frequency != EPSNFrequency::PSN_OnTick)
	{
		float TimerTime = 1.f;
		switch (Frequency)
		{
		case EPSNFrequency::PSN_30Hz:
			TimerTime = 1.f / 30.0;
			break;
		case EPSNFrequency::PSN_60Hz:
			TimerTime = 1.f / 60.0;
			break;
		case EPSNFrequency::PSN_120Hz:
			TimerTime = 1.f / 120.0;
			break;
		default:
			break;
		}

		UE_LOG(LogPSN, Display, TEXT("PSN Will send every %f seconds"), TimerTime);

		// Start timer for Data Packet.
		TimerManager.SetTimer(TimerDataPacket, this, &UPSNSenderSubsystem::SendData, TimerTime, true);
	}

	// Start Info Packet Timer. Hard-Coded at 1Hz
	TimerManager.SetTimer(TimerInfoPacket, this, &UPSNSenderSubsystem::SendInfo, 1.f, true);
	
}

void UPSNSenderSubsystem::AddTracker(FPSNTrackerInfo TrackerInfo)
{
	TrackerInfo.SystemName = ChosenSystemName;
	TrackerMap.Add(FName(*TrackerInfo.Name), FPSNTracker(TrackerInfo));
}

void UPSNSenderSubsystem::UpdateTracker(FName TrackerName, FPSNTrackerData NewData)
{
	FPSNTracker* T = TrackerMap.Find(TrackerName);
	if (T)
	{
		FPSNTrackerInfo NewMeta(T->Info.ID, T->Info.Name);
		FPSNTracker NewTracker(NewMeta, NewData);
		NewTracker.Header.Timestamp = GetTimestamp();

		// Convert from CM to M
		NewTracker.Data.Position *= 0.01;
		NewTracker.Data.TargetPosition *= 0.01;
		TrackerMap.Emplace(TrackerName, NewTracker);
	}
}

void UPSNSenderSubsystem::AddComponentToTrack(FPSNTrackerInfo Meta, USceneComponent* Component)
{
	if (Component)
	{
		ComponentMap.Add(Meta, Component);
	}
}

uint64 UPSNSenderSubsystem::GetTimestamp()
{
	return ((double)UKismetSystemLibrary::GetGameTimeInSeconds(this)) * 1e+6;
}

void UPSNSenderSubsystem::SendData()
{
	BuildTrackerList();
	SenderPtr->SendPSNData(TrackerList, GetTimestamp());
}

void UPSNSenderSubsystem::SendInfo()
{
	BuildTrackerList();
	SenderPtr->SendPSNInfo(TrackerList, GetTimestamp());
}

bool UPSNSenderSubsystem::IsSenderRunning()
{
	return SenderPtr.IsValid();
}

int UPSNSenderSubsystem::FindFreeID(int StartFrom)
{
	BuildTrackerList();
	int Start = StartFrom--;
	bool Checking = true;
	int FoundID = -1;

	while (Checking)
	{
		FoundID = CheckAvailableID(Start++);
		Checking = FoundID < 1;
	}

	return FoundID;
}

bool UPSNSenderSubsystem::GetLocalHostAddress(FString& Address)
{
	if (!Address.IsEmpty() && Address != TEXT("0"))
	{
		return false;
	}

	bool bCanBind = false;
	bool bAppendPort = false;
	if (ISocketSubsystem* SocketSys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM))
	{
		const TSharedPtr<FInternetAddr> Addr = SocketSys->GetLocalHostAddr(*GLog, bCanBind);
		if (Addr.IsValid())
		{
			Address = Addr->ToString(bAppendPort);
			return true;
		}
	}

	return false;
}

void UPSNSenderSubsystem::BuildTrackerList()
{
	// Temporary List for all our trackers, both components and manually added.
	TrackerList.Empty();

	// Push the TrackerMap into the new TrackerList
	TrackerMap.GenerateValueArray(TrackerList);

	// Run the Component system automation
	for (TPair<FPSNTrackerInfo, USceneComponent*> C : ComponentMap)
	{
		if (const USceneComponent* Comp = C.Value)
		{
			FPSNTracker Tracker;
			Tracker.Info = C.Key;
			Tracker.Data.Position = (Comp->GetComponentLocation() * 0.01); // convert from cm to m
			Tracker.Data.Orientation = Comp->GetComponentRotation();
			Tracker.Data.Speed = Comp->GetComponentVelocity();
			Tracker.Data.Acceleration = Comp->GetComponentVelocity();
			Tracker.Data.Status = 1;
			Tracker.Data.TargetPosition = FVector::ZeroVector;
			Tracker.Header.Timestamp = GetTimestamp();
			TrackerList.Add(Tracker);
		}
		// If component is NULL, send a default tracker with 0 status so that we keep the stream consistent
		else
		{
			FPSNTracker Tracker;
			Tracker.Info = C.Key;
			FPSNTrackerData Data;
			Data.Status = 0;
			TrackerList.Add(Tracker);
		}
	}

	// Push the system name into the list. This could be done at other places in a future update
	for (auto T : TrackerList)
	{
		T.Info.SystemName = ChosenSystemName;
	}
}

int UPSNSenderSubsystem::CheckAvailableID(int InID)
{
	for (auto T : TrackerList)
	{
		if (T.Info.ID == InID)
		{
			return -1;
		}
	}
	return InID;
}
