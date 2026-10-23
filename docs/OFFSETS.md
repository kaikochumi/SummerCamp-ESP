# Offsets

Layout used by the tool, against the current **Camp Forest Green** build
of Summer Camp: Unreal Engine 4.18.3, build `4.18.3-38951+SummerCamp`,
main module `SummerCamp-Win64-Shipping.exe`.

Every value below was confirmed twice: first by mapping the engine and
game structures for this build (engine 4.18.3 layout, the game's
USTRUCT/UCLASS fields and globals), then against a live process while
playing. Nothing here is guesswork pointing at random memory.

Everything is also in code: globals and the actor chain in
`src/offsets.h`, the health/stamina constants in `src/game.cpp`, and
the class index table in `src/class_table.h`.

## Globals (module base + value)

| Name           | Offset      | Notes                                        |
|----------------|-------------|----------------------------------------------|
| `GWorld`       | `0x02E0D560`| Entry for the world chain                    |
| `GObjects`     | `0x03199950`| TArray of all UObject chunks                 |
| `GNames`       | `0x00000000`| Names block not mapped; not used             |
| `ProcessEvent` | `0x007AC810`| Not called; kept for reference               |
| `ProcessEvent` VFT index | `0x3F` | Virtual index in uobject vtable      |

## World chain

The ESP walks `GWorld → PersistentLevel → Actors` and filters by class
through `GObjects` class resolution.

| Field                    | Offset | Struct                |
|--------------------------|--------|-----------------------|
| World → Level            | `0x30` | `UWorld::PersistentLevel` |
| World → GameInstance     | `0x140`| `UWorld::OwningGameInstance` |
| World → GameState        | `0xF8` | —                     |
| World → Levels           | `0x110`| TArray                |
| Level → Actors           | `0xA0` | `ULevel::Actors` TArray |
| Actor → RootComponent    | `0x160`| `USceneComponent`     |
| Scene → RelativeLocation | `0x15C`| FVector               |
| Scene → RelativeRotation | `0x168`| FRotator              |
| Scene → LastRenderedFrame| `0x2D8`| frame counter         |
| Scene → bRecentlyRendered| `0x2DC`| visibility bit        |
| World → world time       | `0x394`| float, used for render freshness |

## Character fields

| Field                        | Offset  | Class                   | Source |
|------------------------------|---------|-------------------------|--------|
| Health                       | `0xE58` | `ASCCharacter`          | mapped layout |
| MaxHealth                    | `0xE64` | `ASCCharacter`          | mapped layout |
| Stamina                      | `0x1318`| `ASCCounselorCharacter` | mapped layout |
| MaxStamina                   | `0x1324`| `ASCCounselorCharacter` | mapped layout |
| Mesh component               | `0x3C8` | `ACharacter`            | mapped layout |
| Capsule component            | `0x3D8` | `ACharacter`            | mapped layout |
| Pawn → PlayerController      | `0x3A0` | `APawn`                 | mapped layout |
| PlayerState (name)           | —       | `APawn::PlayerState`    | via actor chain, see `game.cpp` |

## Local player and camera

| Field                              | Offset | Struct                  |
|------------------------------------|--------|-------------------------|
| GameInstance → LocalPlayers        | `0x38` | TArray                  |
| LocalPlayer → PlayerController     | `0x30` | `APlayerController`     |
| LocalPlayer → ViewportClient       | `0x58` | `UGameViewportClient`   |
| PC → LocalPawn (AcknowledgedPawn)  | `0x3E0`| `APawn`                 |
| PC → CameraManager                 | `0x400`| `APlayerCameraManager`  |
| CameraManager → CameraCache        | `0x3E0`| `FCameraCacheEntry`     |
| CameraManager → DefaultFOV         | `0x380`| float                   |
| Cache → POV                        | `0x10` | `FMinimalViewInfo`      |
| POV → Location                     | `0x00` | FVector                 |
| POV → Rotation                     | `0x0C` | FRotator (pitch/yaw/roll)|
| POV → FOV                          | `0x18` | float, horizontal       |

## Mesh and skeleton

| Field                            | Offset  | Source       |
|----------------------------------|---------|--------------|
| SkeletalMesh on the scene comp   | `0x700` | mapped layout |
| Cached component-space transforms| `0x920` | mapped layout |
| Component-space transforms array | `0x710` | memory scan  |
| Per-bone FSkeletalMeshObject data| `0x130` | memory scan  |

The scan-found addresses were located by diffing mesh memory while the
character moved; they have no reflected `UPROPERTY` marker in this
build, so the structure map alone does not make them obvious.
`src/bones.h` maps the 67 bone slots of the character mesh to a
37-segment skeleton drawing.

## How the ESP resolves classes

`GObjects` is a chunked TArray. For every actor the tool resolves its
`UClass` and walks `Class → ClassParent` until a known name shows up in
`src/class_table.h` (an index → class-name table for this build). That
keeps the actor filter alive even when actor classes inherit from
deeper parents.

## When the game updates

Values above will move with any engine or game update. The fastest way
to re-locate them is to map the new build's structures and confirm
against a live process; the scanned fields need a short diff session
(move the character, watch which fields track the transform array).
