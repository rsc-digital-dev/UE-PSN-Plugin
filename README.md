# Posi Stage Net
## An Open Protocol for On-Stage, Live 3D Position Data
## Unreal Engine Integration
### By Daniel Orchard with the Royal Shakespeare Company
[Posi Stage Net Website](https://www.posistage.net/) | [Posi Stage Net Sourcecode](https://github.com/vyv/psn-cpp)

> This plugin is built against PSN version 2.03

The Posi Stage Net (PSN) Integration for Unreal Engine supports both sending and receiving of PSN data.

Both the sender and receiver are contained in a Game Instance Subsystem to ensure a single instance per session.

PSN Info packets are handled internally and sent a 1hz

***

### PSN Sender
The PSN sender is designed to keep a constant list of trackers to send. Using the AddTracker function, you can create new trackers that will be sent. There is no method for removing trackers from the stream, any trackers you wish to update can be done via the UpdateTracker function, and if it is not updated, will send the stale data from the previous update. 

> The PSN sender converts the Position from Unreal Units (cm) to Meters internally

### PSN Receiver
The PSN received is a queued delegate system. You can bind to the OnTrackerReceived event from the subsystem. As a queued system, this event may fire multiple times in a single frame. Its recommended you use the ID or name as a sorting method to update the correct data.

> The PSN receiver converts the Position from meters into Unreal Units (cm)

> Incoming data may come in at a different scale or rotation order compared to the Unreal standard. You may need to scale your *Position* vector or swizzle your *orientation* data to match your source packages transforms.