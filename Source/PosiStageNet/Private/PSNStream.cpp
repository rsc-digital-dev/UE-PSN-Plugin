// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.

#include "PSNStream.h"
#include "PosiStageNet.h"
#include "PSNMessage.h"

#include "PSN/psn_lib.hpp"
#include <sstream>


FPSNStream::FPSNStream()
	: Position(0)
{
}

FPSNStream::FPSNStream(const uint8* InData, int32 InSize)
	: Data(InData, InSize)
	, Position(0)
{
}

FPSNStream::FPSNStream(int32 InSize)
	: Position(0)
{
	Data.AddZeroed(InSize);
}

FPSNStream::FPSNStream(const uint8* InData, int32 InSize, uint8_t InLastFrameID)
	: Data(InData, InSize)
	, Position(0)
	, HeaderFrameID(InLastFrameID)
{
}


void FPSNStream::DecodeToTrackers(TArray<FPSNTracker>& InTrackerMap, ::psn::psn_decoder* Decoder)
{
	check(Decoder);
	InTrackerMap.Empty();

	if (Decoder->decode((ANSICHAR*)Data.GetData(), Data.Num()))
	{
		// Confirm New Frame
		if (Decoder->get_data().header.frame_id != HeaderFrameID)
		{
			HeaderFrameID = Decoder->get_data().header.frame_id;

			const ::psn::tracker_map& recv_trackers = Decoder->get_data().trackers;

			// Header Information
			const FString SystemName = StringToFString(Decoder->get_info().system_name);
			const int FrameID = (int)HeaderFrameID;
			const uint64_t Timestamp = Decoder->get_data().header.timestamp_usec;
			const int TrackerCount = recv_trackers.size();

			// Trackers
			for (auto track = recv_trackers.begin(); track != recv_trackers.end(); ++track)
			{
				const ::psn::tracker& Tracker = track->second;
				InTrackerMap.Add(FPSNTracker(Tracker));
			}
		}
		StreamDataType = Decoder->DataType;
	}
	else
	{
		UE_LOG(LogPSN, Error, TEXT("Failed to decode Data "));
	}
}

void FPSNStream::EncodeToPSN(TArray<FPSNTracker> InTrackerMap, ::psn::psn_encoder* InEncoder, strlist& dataPacket, strlist& InfoPacket, uint64 TimespanMicroseconds, bool bIsHeader)
{
	psn::tracker_map Trackers;
	for (int i = 0; i < InTrackerMap.Num(); i++)
	{
		Trackers.emplace(InTrackerMap[i].Info.ID, InTrackerMap[i].GetAsNativeTracker());
	}

	if (bIsHeader)
	{
		InfoPacket = InEncoder->encode_info(Trackers, TimespanMicroseconds);
	}
	else
	{
		dataPacket = InEncoder->encode_data(Trackers, TimespanMicroseconds);
	}
}

uint8_t FPSNStream::GetHeaderFrameID()
{
	return HeaderFrameID;
}

FString FPSNStream::StringToFString(std::string InString)
{
	return UTF8_TO_TCHAR(InString.c_str());
}

