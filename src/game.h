// game.h — game state reading and world->screen projection.
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "memory.h"
#include "offsets.h"

namespace game {

struct Vec3 {
    float x = 0.f, y = 0.f, z = 0.f;
};

// A character bone, already projected to screen.
struct Bone {
    float sx = 0.f, sy = 0.f;
    int parent = -1;   // -1 root, -2 off screen
};

// An ESP target (here, a counselor).
struct Entity {
    uintptr_t addr = 0;
    std::string name;       // player name (APlayerState::PlayerName)
    Vec3 feet{};            // RootComponent position (capsule center)
    Vec3 head{};            // estimate derived from capsule height
    float half_height = 0.f;
    float distance = 0.f;
    float health = 0.f, max_health = 0.f;
    float stamina = 0.f, max_stamina = 0.f;
    bool in_front = false;
    bool on_screen = false;   // within window bounds
    bool visible = false;      // the game is rendering the character
    float screen_x = 0.f, screen_y = 0.f;
    float screen_h = 0.f;
};

struct Camera {
    Vec3 location{};
    Vec3 rotation{};  // pitch, yaw, roll in degrees
    float fov = 90.f;  // horizontal, degrees
    bool valid = false;
};

class Game {
public:
    // Attaches to the process and builds the class map (address -> name).
    bool attach(mem::Process* proc, const char* module_hint = "summercamp");

    // Re-reads the game state. Cheap: a few reads per call.
    void refresh();

    const Camera& camera() const { return camera_; }
    const std::vector<Entity>& targets() const { return targets_; }
    uintptr_t local_pawn() const { return local_pawn_; }
    const std::string& local_class() const { return local_class_; }
    int actor_count() const { return actor_count_; }
    // How many targets are in front of the camera and inside the screen.
    int visible_count() const { return visible_count_; }
    // Is there a local pawn? It only exists in a match; in the main menu
    // lobby the game creates no character.
    bool in_match() const { return local_pawn_ != 0; }
    bool ready() const { return ready_; }

    // Class name of an object, resolving through the inheritance chain
    // when the class itself is not in the table.
    std::string class_name(uintptr_t obj) const;

    // UE4 FString (TArray<TCHAR>) read and converted to UTF-8.
    std::string read_fstring(uintptr_t addr) const;

    // Character skeleton, already projected to screen. Returns the number of
    // bones read.
    int skeleton(const Entity& e, std::vector<Bone>& out) const;

    // World->screen projection. Returns false if the point is behind the camera.
    bool world_to_screen(const Vec3& world, float& out_x, float& out_y) const;

    void set_viewport(int w, int h) { view_w_ = w; view_h_ = h; }

    // Class the ESP hunts. By default it is auto-detected from the local
    // character: if you play as Jason, targets are counselors;
    // if you play as a counselor, the target is Jason.
    void set_target_class(const std::string& name) {
        target_class_ = name;
        auto_target_ = false;
    }
    const std::string& target_class() const { return target_class_; }
    bool auto_target() const { return auto_target_; }
    void set_auto_target(bool on) { auto_target_ = on; }

private:
    void build_class_map(HANDLE h);
    std::string ancestor_name(HANDLE h, uintptr_t cls) const;
    // Detects the target class from the local player's side.
    void detect_target_class();

    mem::Process* proc_ = nullptr;
    uintptr_t base_ = 0;
    bool ready_ = false;

    uintptr_t world_ = 0;
    uintptr_t level_ = 0;
    uintptr_t actors_data_ = 0;
    int actor_count_ = 0;
    uintptr_t local_pawn_ = 0;
    std::string local_class_;

    Camera camera_;
    std::vector<Entity> targets_;
    int visible_count_ = 0;
    int view_w_ = 1920, view_h_ = 1080;
    std::string target_class_ = "SummerCamp.SCCounselorCharacter";
    bool auto_target_ = true;
    float world_time_ = 0.f;

    // UClass address -> name, built once at attach
    std::unordered_map<uintptr_t, std::string> class_map_;
};

}  // namespace game
