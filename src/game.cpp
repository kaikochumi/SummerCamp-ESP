// game.cpp — game state reading and world->screen projection.
#include "game.h"

#include <cmath>
#include <cstring>
#include <vector>

#include "bones.h"
#include "class_table.h"

namespace game {
namespace {

constexpr float kDeg2Rad = 0.017453292519943295f;
constexpr uintptr_t kSuperStruct = 0x30;   // UStruct::SuperStruct
constexpr uintptr_t kClassPrivate = 0x10;   // UObject::ClassPrivate
constexpr uintptr_t kCapsuleHalfHeight = 0x6F8;
constexpr uintptr_t kItemSize = 0x18;       // sizeof(FUObjectItem)
constexpr uintptr_t kPawnPlayerState = 0x388;    // APawn::PlayerState
constexpr uintptr_t kPsPlayerName = 0x370;      // APlayerState::PlayerName
constexpr uintptr_t kCharHealth = 0xE58;        // ASCCharacter::Health
constexpr uintptr_t kCharMaxHealth = 0xE64;     // ASCCharacter::MaxHealth
constexpr uintptr_t kCounselorStamina = 0x1318; // ASCCounselorCharacter
constexpr uintptr_t kCounselorStaminaMax = 0x1324;

}  // namespace

void Game::build_class_map(HANDLE h) {
    class_map_.clear();
    const uintptr_t arr = base_ + Offsets::GObjects;
    const uintptr_t objects = proc_->read<uintptr_t>(arr);
    const int32_t num = proc_->read<int32_t>(arr + 0x0C);
    if (!objects || num <= 0) return;

    for (int i = 0; i < ClassTable::kCount; ++i) {
        const auto& e = ClassTable::kEntries[i];
        if (static_cast<int32_t>(e.index) >= num) continue;
        const uintptr_t addr =
            proc_->read<uintptr_t>(objects + e.index * kItemSize);
        if (!addr) continue;
        class_map_[addr] = e.name;
    }
}

std::string Game::ancestor_name(HANDLE h, uintptr_t cls) const {
    uintptr_t cur = cls;
    for (int depth = 0; depth < 24 && cur; ++depth) {
        auto it = class_map_.find(cur);
        if (it != class_map_.end()) return it->second;
        cur = proc_->read<uintptr_t>(cur + kSuperStruct);
    }
    return {};
}

std::string Game::class_name(uintptr_t obj) const {
    if (!obj) return {};
    const uintptr_t cls = proc_->read<uintptr_t>(obj + kClassPrivate);
    if (!cls) return {};
    auto it = class_map_.find(cls);
    if (it != class_map_.end()) return it->second;
    return ancestor_name(proc_->handle(), cls);
}

bool Game::attach(mem::Process* proc, const char* module_hint) {
    proc_ = proc;
    if (!proc_ || !proc_->attach(module_hint)) return false;
    base_ = proc_->base();
    build_class_map(proc_->handle());
    ready_ = true;
    return true;
}

std::string Game::read_fstring(uintptr_t addr) const {
    if (!addr) return {};
    const uintptr_t data = proc_->read<uintptr_t>(addr);
    const int32_t count = proc_->read<int32_t>(addr + 8);
    if (!data || count <= 0 || count > 128) return {};

    std::vector<uint16_t> buf(static_cast<size_t>(count));
    if (!proc_->read(data, buf.data(), static_cast<size_t>(count) * 2)) return {};

    // UTF-16LE to UTF-8
    std::string out;
    out.reserve(static_cast<size_t>(count));
    for (const uint16_t c : buf) {
        if (c == 0) continue;
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
        } else if (c < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (c >> 6)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | (c >> 12)));
            out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return out;
}

void Game::detect_target_class() {
    if (!local_pawn_) return;
    uintptr_t cur = proc_->read<uintptr_t>(local_pawn_ + kClassPrivate);
    for (int depth = 0; depth < 24 && cur; ++depth) {
        auto it = class_map_.find(cur);
        if (it != class_map_.end()) {
            const std::string& n = it->second;
            // local player's side defines the target's side
            if (n == "SummerCamp.SCKillerCharacter") {
                target_class_ = "SummerCamp.SCCounselorCharacter";
                return;
            }
            if (n == "SummerCamp.SCCounselorCharacter") {
                target_class_ = "SummerCamp.SCKillerCharacter";
                return;
            }
        }
        cur = proc_->read<uintptr_t>(cur + kSuperStruct);
    }
}

void Game::refresh() {
    if (!ready_) return;

    world_ = proc_->read<uintptr_t>(base_ + Offsets::GWorld);
    level_ = world_ ? proc_->read<uintptr_t>(world_ + Offsets::OFF_WORLD_LEVEL) : 0;
    actors_data_ = level_ ? proc_->read<uintptr_t>(level_ + Offsets::OFF_LEVEL_ACTORS) : 0;
    actor_count_ = level_
        ? static_cast<int>(proc_->read<uint32_t>(level_ + Offsets::OFF_LEVEL_ACTORS + 8))
        : 0;

    // camera
    camera_ = Camera{};
    const uintptr_t gi = world_ ? proc_->read<uintptr_t>(world_ + Offsets::OFF_WORLD_GI) : 0;
    const uintptr_t lps = gi ? proc_->read<uintptr_t>(gi + Offsets::OFF_GI_LOCALPLAYERS) : 0;
    const uintptr_t lp = lps ? proc_->read<uintptr_t>(lps) : 0;
    const uintptr_t pc = lp ? proc_->read<uintptr_t>(lp + Offsets::OFF_PLAYER_PC) : 0;
    local_pawn_ = pc ? proc_->read<uintptr_t>(pc + Offsets::OFF_PC_ACKPAWN) : 0;
    if (local_pawn_ && local_class_.empty())
        local_class_ = class_name(local_pawn_);
    if (auto_target_) detect_target_class();

    // world time: reference to know who is being rendered
    const uintptr_t gs =
        world_ ? proc_->read<uintptr_t>(world_ + Offsets::OFF_WORLD_GAMESTATE) : 0;
    world_time_ = gs ? proc_->read<float>(gs + Offsets::OFF_WORLD_TIME) : 0.f;

    const uintptr_t cam = pc ? proc_->read<uintptr_t>(pc + Offsets::OFF_PC_CAMMGR) : 0;
    if (cam) {
        const uintptr_t pov = cam + Offsets::OFF_CAM_CACHE + Offsets::OFF_CACHE_POV;
        camera_.location = proc_->read<Vec3>(pov + Offsets::OFF_POV_LOCATION);
        camera_.rotation = proc_->read<Vec3>(pov + Offsets::OFF_POV_ROTATION);
        camera_.fov = proc_->read<float>(pov + Offsets::OFF_POV_FOV);
        camera_.valid = camera_.fov > 1.f && camera_.fov < 179.f;
    }

    // targets
    targets_.clear();
    visible_count_ = 0;
    if (!actors_data_ || actor_count_ <= 0 || !camera_.valid) return;
    if (actor_count_ > 4000) actor_count_ = 4000;

    for (int i = 0; i < actor_count_; ++i) {
        const uintptr_t actor =
            proc_->read<uintptr_t>(actors_data_ + i * sizeof(uintptr_t));
        if (!actor) continue;

        const uintptr_t cls = proc_->read<uintptr_t>(actor + kClassPrivate);
        if (!cls) continue;

        std::string name;
        auto it = class_map_.find(cls);
        if (it != class_map_.end()) {
            name = it->second;
        } else {
            name = ancestor_name(proc_->handle(), cls);
        }
        if (name != target_class_) continue;

        const uintptr_t root = proc_->read<uintptr_t>(actor + Offsets::OFF_ACTOR_ROOT);
        if (!root) continue;
        const Vec3 center = proc_->read<Vec3>(root + Offsets::OFF_SCENE_RELLOC);

        float half = 90.f;  // fallback: character ~180cm tall
        const uintptr_t capsule = proc_->read<uintptr_t>(actor + Offsets::OFF_CHAR_CAPSULE);
        if (capsule) {
            const float h = proc_->read<float>(capsule + kCapsuleHalfHeight);
            if (h > 10.f && h < 400.f) half = h;
        }

        Entity e;
        e.addr = actor;
        e.feet = center;
        e.half_height = half;
        e.head = {center.x, center.y, center.z + half};

        // player name
        const uintptr_t ps = proc_->read<uintptr_t>(actor + kPawnPlayerState);
        if (ps) e.name = read_fstring(ps + kPsPlayerName);

        // health and stamina
        e.health = proc_->read<float>(actor + kCharHealth);
        e.max_health = proc_->read<float>(actor + kCharMaxHealth);
        e.stamina = proc_->read<float>(actor + kCounselorStamina);
        e.max_stamina = proc_->read<float>(actor + kCounselorStaminaMax);
        if (e.max_health <= 0.f || e.max_health > 100000.f) e.max_health = 0.f;
        if (e.max_stamina <= 0.f || e.max_stamina > 100000.f) e.max_stamina = 0.f;

        // dead characters are skipped by the ESP: a lifeless body is useless.
        // Dead chars read 0 with a valid max (evidence: probe 0/100 with the
        // pelvis lying at z=13); the living always have health > 0.
        if (e.max_health > 0.f && e.health <= 0.f) continue;

        // is the game drawing this character on screen? the world time
        // works as a clock. If the clock is not running, everything
        // counts as visible (open failure mode).
        const float on_screen_time =
            proc_->read<float>(root + Offsets::OFF_SCENE_ONSCREEN);
        e.visible = world_time_ <= 0.f || (world_time_ - on_screen_time) < 0.3f;

        const float dx = center.x - camera_.location.x;
        const float dy = center.y - camera_.location.y;
        const float dz = center.z - camera_.location.z;
        e.distance = std::sqrt(dx * dx + dy * dy + dz * dz);

        float fx, fy;
        const Vec3 bottom{center.x, center.y, center.z - half};
        if (world_to_screen(bottom, fx, fy)) {
            e.in_front = true;
            e.screen_x = fx;
            e.screen_y = fy;
            float hx, hy;
            if (world_to_screen(e.head, hx, hy)) {
                e.screen_h = std::fabs(hy - fy);
                // visible = within the game window bounds
                e.on_screen = e.screen_h > 0.5f && fx >= -60.f &&
                              fx <= static_cast<float>(view_w_) + 60.f &&
                              fy >= -60.f && fy <= static_cast<float>(view_h_) + 60.f;
            }
        }
        if (e.on_screen) ++visible_count_;
        targets_.push_back(e);
    }
}

// The live skeleton pose lives in USkeletalMeshComponent, in a TArray
// that has no reflected UPROPERTY marker in this build. Offset 0x710 was
// found by scanning: it is the array of 67 FTransforms in component
// space, which gives the final position of each bone.
//
// The pose is in mesh space, and the mesh origin is NOT the character origin:
// it is "glued" to the root with its own offset (it lands at the feet) and
// usually carries a base rotation. Without composing that, the skeleton comes
// out half a capsule above the ground and points in a fixed direction,
// ignoring the character's heading. The bone order is fixed in the character,
// so the ESP bone code uses the src/bones.h indices and needs no parent table.
int Game::skeleton(const Entity& e, std::vector<Bone>& out) const {
    out.clear();
    const uintptr_t mesh = proc_->read<uintptr_t>(e.addr + Offsets::OFF_CHAR_MESH);
    if (!mesh) return 0;

    const uintptr_t arr = mesh + Offsets::OFF_COMPONENT_TRANSFORMS;
    const uintptr_t data = proc_->read<uintptr_t>(arr);
    const int32_t count = proc_->read<int32_t>(arr + 8);
    if (!data || count < bones::kCount) return 0;

    // world-out transform of the mesh component: root position + root
    // rotation applied to the mesh RelativeLocation.
    const uintptr_t root = proc_->read<uintptr_t>(e.addr + Offsets::OFF_ACTOR_ROOT);
    if (!root) return 0;
    const Vec3 root_rot = proc_->read<Vec3>(root + Offsets::OFF_SCENE_RELROT);
    const Vec3 mesh_rel = proc_->read<Vec3>(mesh + Offsets::OFF_SCENE_RELLOC);
    const Vec3 mesh_rot = proc_->read<Vec3>(mesh + Offsets::OFF_SCENE_RELROT);

    const float base_rad = root_rot.y * kDeg2Rad;
    const float bc = std::cos(base_rad), bs = std::sin(base_rad);
    const float bone_rad = (root_rot.y + mesh_rot.y) * kDeg2Rad;
    const float c = std::cos(bone_rad), s = std::sin(bone_rad);

    const float ox = e.feet.x + mesh_rel.x * bc - mesh_rel.y * bs;
    const float oy = e.feet.y + mesh_rel.x * bs + mesh_rel.y * bc;
    const float oz = e.feet.z + mesh_rel.z;

    out.resize(bones::kCount);
    for (int i = 0; i < bones::kCount; ++i) {
        Bone& b = out[static_cast<size_t>(i)];
        b.parent = -1;
        // FTransform: Translation at +0x10, size 0x30
        const float lx = proc_->read<float>(data + i * 0x30 + 0x10);
        const float ly = proc_->read<float>(data + i * 0x30 + 0x14);
        const float lz = proc_->read<float>(data + i * 0x30 + 0x18);
        const Vec3 world{ox + lx * c - ly * s, oy + lx * s + ly * c, oz + lz};
        float sx = 0.f, sy = 0.f;
        if (!world_to_screen(world, sx, sy)) {
            b.parent = -2;
            continue;
        }
        b.sx = sx;
        b.sy = sy;
    }
    return bones::kCount;
}

bool Game::world_to_screen(const Vec3& world, float& out_x, float& out_y) const {
    if (!camera_.valid) return false;

    const float pitch = camera_.rotation.x * kDeg2Rad;
    const float yaw = camera_.rotation.y * kDeg2Rad;
    const float roll = camera_.rotation.z * kDeg2Rad;
    const float sp = std::sin(pitch), cp = std::cos(pitch);
    const float sy = std::sin(yaw), cy = std::cos(yaw);
    const float sr = std::sin(roll), cr = std::cos(roll);

    // FRotationMatrix: row 0 = X (forward), 1 = Y (right), 2 = Z (up)
    const float fx = cp * cy;
    const float fy = cp * sy;
    const float fz = sp;
    const float rx = sr * sp * cy - cr * sy;
    const float ry = sr * sp * sy + cr * cy;
    const float rz = -sr * cp;
    const float ux = -(cr * sp * cy + sr * sy);
    const float uy = cy * sr - cr * sp * sy;
    const float uz = cp * cr;

    const float dx = world.x - camera_.location.x;
    const float dy = world.y - camera_.location.y;
    const float dz = world.z - camera_.location.z;

    const float forward = dx * fx + dy * fy + dz * fz;
    if (forward <= 1.f) return false;  // behind the camera

    const float right = dx * rx + dy * ry + dz * rz;
    const float up = dx * ux + dy * uy + dz * uz;

    const float focal = (view_w_ * 0.5f) /
                        std::tan(camera_.fov * 0.5f * kDeg2Rad);
    out_x = view_w_ * 0.5f + right * focal / forward;
    out_y = view_h_ * 0.5f - up * focal / forward;
    return true;
}

}  // namespace game
