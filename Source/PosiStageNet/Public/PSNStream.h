// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include <string.h>
#include <list>

struct FTracker;

// Define std list of string as a typedef
typedef ::std::list <::std::string> strlist;

/*
* PSNStream is a class designed to handle the conversion from native PSN to our Unreal Trackers, or vice versa.
*/

class FPSNStream
{
public:

	// ctor - null
	FPSNStream();

	// ctor - with data
	FPSNStream(const uint8* InData, int32 InSize);

	// ctor - with data and a last frame ID
	FPSNStream(const uint8* InData, int32 InSize, uint8_t InLastFrameID);

	// ctor - sized only, null data
	FPSNStream(int32 InSize);

	// Decode Tracker Map. Requires Stream to be made with the data ctor so it has size and data ready to decode. 
	void DecodeToTrackers(TArray<FPSNTracker>& InTrackerMap, ::psn::psn_decoder* Decoder);

	// Encode Tracker Map, return data and info packets
	void EncodeToPSN(TArray<FPSNTracker> InTrackerMap, ::psn::psn_encoder* InEncoder, strlist& dataPacket, strlist& InfoPacket, uint64 TimespanMicroseconds, bool bIsHeader);

	uint8_t GetHeaderFrameID();

	EPSNPacketType StreamDataType;

private:

	inline FString StringToFString(std::string InString);

	/** Stream data. */
	TArray<uint8> Data;

	/** Current buffer position. */
	int32 Position;

	uint8_t HeaderFrameID;

};
