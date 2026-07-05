// ============================================================================
//  Enemy.h  —  the data for one moving enemy
//
//  This is a pure data type (no functions), so it lives ENTIRELY in a header
//  with no matching .cpp file. There is nothing to compile separately, so it
//  can never cause a linker error. Rule of thumb: data-only structs are
//  header-only.
// ============================================================================
#pragma once   // include this file's contents at most once per .cpp

// An enemy doesn't store its own (col,row). Instead it stores how far along
// the pre-computed path it is. Its actual cell is always path[pathIndex].
struct Enemy {
    int  pathIndex;    // position along the path. Negative = not spawned yet
    // (a countdown that staggers enemies so they don't stack).
    int  health;       // hit points. Towers reduce this; at 0 the enemy dies
    // and stops moving, being drawn, and being targeted.
    bool reachedBase;  // set true once it arrives at the base, so each arrival
    // is only counted against the player's lives one time.
};