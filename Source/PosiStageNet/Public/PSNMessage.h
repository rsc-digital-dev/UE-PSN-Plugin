// Copyright 2021 Royal Shakespeare Company. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "PSN/psn_encoder.hpp"
#include "PSN/psn_decoder.hpp"
#include "PSNMessage.generated.h"

UENUM(BlueprintType)
enum class EPSNFrequency : uint8
{
	PSN_30Hz		UMETA(DisplayName = "30Hz"),
	PSN_60Hz		UMETA(DisplayName = "60Hz"),
	PSN_120Hz		UMETA(DisplayName = "120Hz"),
	PSN_OnTick      UMETA(DisplayName = "Tick"),
};

USTRUCT(BlueprintType)
struct FPSNTrackerData
{
	GENERATED_BODY()
public:

	/** Tracker Position */
	UPROPERTY(BlueprintReadWrite, Category = "PSN")
	FVector Position;

	/** Tracker Speed */
	UPROPERTY(BlueprintReadWrite, Category = "PSN")
	FVector Speed;

	/** Tracker Rotation */
	UPROPERTY(BlueprintReadWrite, Category = "PSN")
	FRotator Orientation;

	/** Tracker Status */
	UPROPERTY(BlueprintReadWrite, Category = "PSN")
	float Status;

	/** Tracker Acceleration */
	UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category = "PSN")
	FVector Acceleration;

	/** Tracker Target Position */
	UPROPERTY(BlueprintReadWrite, AdvancedDisplay, Category = "PSN")
	FVector TargetPosition;

	FPSNTrackerData()
	{
		Position = FVector::ZeroVector;
		Speed = FVector::ZeroVector;
		Orientation = FRotator::ZeroRotator;
		Status = 0;
		Acceleration = FVector::ZeroVector;
		TargetPosition = FVector::ZeroVector;
	};

	bool operator==(const FPSNTrackerData& Other) const
	{
		return Position == Other.Position &&
			Speed == Other.Speed &&
			Orientation == Other.Orientation &&
			Status == Other.Status &&
			Acceleration == Other.Acceleration &&
			TargetPosition == Other.TargetPosition;
	}

	// Convert psn float3 into FVector
	FVector Float3ToFVector(psn::float3 inFloat3)
	{
		return FVector(inFloat3.x, inFloat3.y, inFloat3.z);
	}

	// Convert FVector into PSN's float3 format
	psn::float3 VectorToFloat3(FVector V)
	{
		return psn::float3(V.X, V.Y, V.Z);
	}
};

FORCEINLINE uint32 GetTypeHash(const FPSNTrackerData& Key)
{
	return HashCombine(
		HashCombine(GetTypeHash(Key.Position), GetTypeHash(Key.Speed)),
		GetTypeHash(Key.TargetPosition));
}


USTRUCT(BlueprintType)
struct FPSNTrackerInfo
{
	GENERATED_BODY()
public:

	/** Tracker ID */
	UPROPERTY(BlueprintReadWrite, Category = "PSN", meta=(ClampMin=1))
	int ID = 1;

	/** Tracker Name */
	UPROPERTY(BlueprintReadWrite, Category = "PSN")
	FString Name;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, AdvancedDisplay, Category = "PSN")
	FString SystemName;

	// ctor - null
	FPSNTrackerInfo()
	{
		ID = 1;
		Name = FString();
	};

	FPSNTrackerInfo(int InID, FString InName)
	{
		ID = InID;
		Name = InName;
		SystemName = FString();
	}

	FPSNTrackerInfo(int InID, FString InName, FString InSystemName)
	{
		ID = InID;
		Name = InName;
		SystemName = InSystemName;
	}

	bool operator==(const FPSNTrackerInfo& Other) const
	{
		return ID == Other.ID && Name == Other.Name;
	}

};

FORCEINLINE uint32 GetTypeHash(const FPSNTrackerInfo& Key)
{
	return HashCombine(Key.ID, GetTypeHash(Key.Name));
}

// Header Data for Tracker, For use with the Receiver only, not for use with sender.
USTRUCT(BlueprintType)
struct FPSNTrackerHeader
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PSN")
	int FrameID;

	/** Tracker Timestamp  */
	UPROPERTY(BlueprintReadOnly, Category = "PSN")
	int64 Timestamp;

	FPSNTrackerHeader()
	{
		FrameID = 0;
		Timestamp = 0;
	}

	FPSNTrackerHeader(int InFrameID, uint64 InTimestamp)
	{
		FrameID = InFrameID;
		Timestamp = (int64)InTimestamp;
	}

	bool operator==(const FPSNTrackerHeader& Other) const
	{
		return FrameID == Other.FrameID && Timestamp == Other.Timestamp;
	}

};

FORCEINLINE uint32 GetTypeHash(const FPSNTrackerHeader& Key)
{
	return HashCombine(GetTypeHash(Key.FrameID), GetTypeHash(Key.Timestamp));
}

USTRUCT(BlueprintType)
struct FPSNTracker
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "PSN")
	FPSNTrackerHeader Header;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="PSN")
	FPSNTrackerInfo Info;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "PSN")
	FPSNTrackerData Data;

	// Constructor
	FPSNTracker()
	{
		Info = FPSNTrackerInfo();
		Data = FPSNTrackerData();
	};

	// Constructor for Info Only
	FPSNTracker(FPSNTrackerInfo InInfo)
	{
		Info = InInfo;
		Data = FPSNTrackerData();
	}

	FPSNTracker(FPSNTrackerInfo InMeta, FPSNTrackerData InData)
	{
		Info = InMeta;
		Data = InData;
	}

	bool operator==(const FPSNTracker& Other) const
	{
		return Info == Other.Info && Data == Other.Data;
	}

	// Convert psn float3 into FVector
	FORCEINLINE FVector Conv_Float3ToUnrealVector(psn::float3 inFloat3)
	{
		return FVector(inFloat3.x, inFloat3.y, inFloat3.z);
	}

	// Convert FVector into PSN's float3 format
	FORCEINLINE psn::float3 Conv_UnrealVectorToFloat3(FVector V)
	{
		return psn::float3(V.X, V.Y, V.Z);
	}

	// ctor - native to unreal
	FPSNTracker(psn::tracker InTracker)
	{
		Info.ID = InTracker.get_id();
		Info.Name = InTracker.get_name().c_str();

		Data.Position = Conv_Float3ToUnrealVector(InTracker.get_pos());
		Data.Speed = Conv_Float3ToUnrealVector(InTracker.get_speed());
		Data.Orientation = Conv_Float3ToUnrealVector(InTracker.get_ori()).Rotation(); // might be broken!
		Data.Status = InTracker.get_status();
		Data.Acceleration = Conv_Float3ToUnrealVector(InTracker.get_accel());
		Data.TargetPosition = Conv_Float3ToUnrealVector(InTracker.get_target_pos());
		Header.Timestamp = InTracker.get_timestamp();
	}

	// Export Tracker as Native PSN::Tracker
	psn::tracker GetAsNativeTracker()
	{
		psn::tracker NewTracker = psn::tracker(Info.ID, TCHAR_TO_UTF8(*Info.Name));

		NewTracker.set_pos(Conv_UnrealVectorToFloat3(Data.Position));
		NewTracker.set_speed(Conv_UnrealVectorToFloat3(Data.Speed));
		NewTracker.set_ori(Conv_UnrealVectorToFloat3(Data.Orientation.Vector()));
		NewTracker.set_status(Data.Status);
		NewTracker.set_accel(Conv_UnrealVectorToFloat3(Data.Acceleration));
		NewTracker.set_target_pos(Conv_UnrealVectorToFloat3(Data.TargetPosition));
		NewTracker.set_timestamp(Header.Timestamp);
		return NewTracker;
	}


};

FORCEINLINE uint32 GetTypeHash(const FPSNTracker& Key)
{
	return HashCombine(GetTypeHash(Key.Info), GetTypeHash(Key.Data));
}

