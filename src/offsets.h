// Memory layout of the current build. Values were mapped from the
// engine/game structures and confirmed against a live process; fields
// marked as scanned were found by memory diffing during play.
#pragma once

namespace Offsets {

// globals (module base + offset)
constexpr int GWorld = 0x02E0D560;
constexpr int GObjects = 0x03199950;
constexpr int GNames = 0x00000000;
constexpr int ProcessEvent = 0x007AC810;
constexpr int ProcessEventIdx = 0x3F;

// ESP chain
constexpr int OFF_WORLD_LEVEL = 0x30;
constexpr int OFF_WORLD_GI = 0x140;
constexpr int OFF_WORLD_LEVELS = 0x110;
constexpr int OFF_WORLD_GAMESTATE = 0xF8;
constexpr int OFF_LEVEL_ACTORS = 0xA0;
constexpr int OFF_ACTOR_ROOT = 0x160;
constexpr int OFF_SCENE_RELLOC = 0x15C;
constexpr int OFF_SCENE_RELROT = 0x168;
constexpr int OFF_SCENE_LASTRENDER = 0x2D8;
constexpr int OFF_SCENE_ONSCREEN = 0x2DC;
constexpr int OFF_SKELETAL_MESH = 0x700;
constexpr int OFF_CACHED_TRANSFORMS = 0x920;
constexpr int OFF_WORLD_TIME = 0x394;
constexpr int OFF_GI_LOCALPLAYERS = 0x38;
constexpr int OFF_PLAYER_PC = 0x30;
constexpr int OFF_LP_VIEWPORTCLIENT = 0x58;
constexpr int OFF_PC_ACKPAWN = 0x3E0;
constexpr int OFF_PC_CAMMGR = 0x400;
constexpr int OFF_CAM_CACHE = 0x3E0;
constexpr int OFF_CAM_FOV = 0x380;
constexpr int OFF_PAWN_CONTROLLER = 0x3A0;
constexpr int OFF_CHAR_MESH = 0x3C8;
constexpr int OFF_CHAR_CAPSULE = 0x3D8;

// camera (FMinimalViewInfo inside FCameraCacheEntry)
constexpr int OFF_CACHE_POV = 0x10;
constexpr int OFF_POV_LOCATION = 0x0;
constexpr int OFF_POV_ROTATION = 0xC;
constexpr int OFF_POV_FOV = 0x18;

// found by scanning (no reflected UPROPERTY marker in this build)
constexpr int OFF_COMPONENT_TRANSFORMS = 0x710;
constexpr int OFF_REF_BONE_POSE = 0x130;

}  // namespace Offsets
