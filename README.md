# Posi Stage Net
## An Open Protocol for On-Stage, Live 3D Position Data
## Unreal Engine Integration
### By Daniel Orchard with the Royal Shakespeare Company
[Posi Stage Net Website](https://www.posistage.net/) | [Posi Stage Net Sourcecode](https://github.com/vyv/psn-cpp)

> This plugin is built against PSN version 2.03

> For commercial licensing please contact digital.dev@rsc.org.uk

The Posi Stage Net (PSN) Integration for Unreal Engine supports both sending and receiving of PSN data.

Both the sender and receiver are contained in a Game Instance Subsystem to ensure a single instance per session.

PSN Info packets are handled internally and sent a 1hz

***

### PSN Sender
The PSN sender is designed to keep a constant list of trackers to send. Using the AddTracker function, you can create new trackers that will be sent. There is no method for removing trackers from the stream, any trackers you wish to update can be done via the UpdateTracker function, and if it is not updated, will send the stale data from the previous update. 

> The PSN sender converts the Position from Unreal Units (cm) to Meters internally

![PSN Sender Overview](Docs/Images/PSN_Sender01.png?raw=true "PSN Sender Blueprint Nodes Overview")

### PSN Receiver
The PSN received is a queued delegate system. You can bind to the OnTrackerReceived event from the subsystem. As a queued system, this event may fire multiple times in a single frame. Its recommended you use the ID or name as a sorting method to update the correct data.

> The PSN receiver converts the Position from meters into Unreal Units (cm)

> Incoming data may come in at a different scale or rotation order compared to the Unreal standard. You may need to scale your *Position* vector or swizzle your *orientation* data to match your source packages transforms.

![PSN Receiver Node Overview](Docs/Images/PSN_Receiver01.png?raw=true "PSN Receiver Blueprint Node Overview")

### PSN Helper,
The PSN Helper is designed to provide an easy way to add location, rotation and scale offsets, as well as swapping X,Y and Z.
> There is a pre-defined one for MA Lighting consoles that swaps X and Y so the co-ordinate spaces are aligned.

> It is located in /Blueprints/Helpers

![PSN Helper Node Overview](Docs/Images/PSN_Helper.png?raw=true "PSN Helper Blueprint Node Overview")



### Getting Started

After downloading and adding the plugin to your project/plugins folder (create if necessary), restart the editor and you will will see the PSN plugin in your plugins list. 

The plugin ships with 2 demo maps and some example blueprint setups. Check out the plugins content folder for these:
```/PosiStageNet/Maps/```


![PSN Sender Map](Docs/Images/Demo_Cubes.gif?raw=false "PSN Sender Map")

![PSN Received](Docs/Images/Demo_GrandMA3.gif?raw=false "PSN Received by external device")
