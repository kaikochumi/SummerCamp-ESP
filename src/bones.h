// bones.h — the 13th character skeleton.
//
// The live pose (component space) is at USkeletalMeshComponent + 0x710,
// with 67 bones. Bone 0 (root) sits at the character's FEET, the pelvis is
// bone 1 (z ~85 when standing) and the head is bone 5 (z ~132).
//
// Bone order read from a counselor STANDING (probe_em_pe.py):
//
//    0       root, at the feet
//    1..5    pelvis, spine, chest, head
//    6..9    shoulder, upper arm, forearm, hand  (left)
//   10..25   left hand fingers (+ twist copied from 8)
//   26..27   left shoulder twist, neck/jaw
//   28..31   shoulder, upper arm, forearm, hand  (right)
//   32..48   right hand fingers + copied twists
//   49..51   hair/top of head
//   52..57   hip, knee, ankle, (dup), foot, toe (left)
//   58..65   same block for the right leg (+ twists)
//   66       copy of the root
//
// EDGES: the root (0) does NOT join the drawing — with it the "legs" would
// go foot -> hip -> knee and form an M. Everything starts at the pelvis (1).
#pragma once

namespace bones {

// (start, end) of each drawn segment.
struct Edge {
    int a;
    int b;
};

constexpr Edge kEdges[] = {
    // spine: pelvis up to head
    {1, 2}, {2, 3}, {3, 4}, {4, 5},
    // chest -> shoulders
    {4, 6}, {4, 28},
    // left arm: shoulder, upper arm, forearm, hand
    {6, 7}, {7, 8}, {8, 9},
    // right arm
    {28, 29}, {29, 30}, {30, 31},
    // left leg: hip, knee, ankle, foot
    {1, 52}, {52, 53}, {53, 54}, {54, 56},
    // right leg
    {1, 59}, {59, 60}, {60, 61}, {61, 63},
};

constexpr int kEdgeCount = sizeof(kEdges) / sizeof(kEdges[0]);

// Number of bones in the character.
constexpr int kCount = 67;

}  // namespace bones
