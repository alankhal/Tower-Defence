# Terminal Tower Defence

A tower defence game written in plain C++17 with no external libraries. The
board is drawn as ASCII in the terminal, and the whole match runs as two nested
loops: an outer loop over waves, and an inner tick loop that moves enemies and
fires towers.

Enemies walk from the spawn on the left to the base on the right. Between waves
the game pauses so you can build and upgrade towers, which reshapes the maze the
ground enemies have to walk through. Every route is worked out with a
breadth-first search, and that same search doubles as the rule that stops you
sealing the base off.

## The board

```
     0  1  2  3  4  5  6  7
  0  .  .  .  .  .  .  .  .
  1  .  .  .  .  .  .  .  .
  2  .  .  .  .  .  .  .  .
  3  o  o  *  o  o  .  .  .
  4  ^  .  . T1  *  *  *  B
  5  .  .  . T1  .  .  .  .
  6  .  .  .  .  .  .  .  .
  7  .  .  .  .  .  .  .  .

  S spawn    B base    * walking route
  T1/T2/T3 cannon (ground only)    A1/A2/A3 anti-air (flying only)
  o basic    > fast    ^ flying (ignores towers, immune to cannons)
```

Two cannons at column 3 have pushed the walking route up into row 3, so the
ground enemies (`o`) are taking the detour. The flyer (`^`) is not: it is
crossing row 4 in a straight line and will pass straight over the towers.

## Building it

**Visual Studio (solution)** — open `TowerDefence.sln` and press F5. If Visual
Studio offers to retarget the project to your installed toolset, accept.

**Visual Studio (folder)** — File > Open > Folder and pick this directory.
Visual Studio reads `CMakeLists.txt` and configures the project on its own;
choose `TowerDefence.exe` as the startup item.

**Command line** — any platform:

```
cmake -S . -B build
cmake --build build
```

The code is standard C++17 with no dependencies. It builds clean under
`/W4` on MSVC and `-Wall -Wextra -pedantic` on GCC and Clang.

## Playing it

During each build phase:

| Command | Effect |
| --- | --- |
| `b <col> <row>` | Build a cannon (5 gold) — shoots ground enemies only |
| `a <col> <row>` | Build an anti-air tower (6 gold) — shoots flying enemies only |
| `u <col> <row>` | Upgrade either kind of tower (4 gold) |
| `s` | Start the wave |

You start with 15 lives and 12 gold, and earn gold for every enemy destroyed
and every wave survived. Five waves, each larger and tougher than the last.

Towers gain range and damage with each of their three levels (range 2/3/4,
damage 3/6/9), so upgrading an existing tower and building a new one are
genuinely different bets.

## Enemy types

| Type | Symbol | Speed | Behaviour |
| --- | --- | --- | --- |
| Basic | `o` | Moves every 3 ticks | Most health. Walks the maze. |
| Fast | `>` | Moves every tick | Less health, but spends far less time under fire. |
| Flying | `^` | Moves every 2 ticks | Ignores towers entirely and flies the direct line. Cannons cannot touch it. |

Each wave rolls its enemies at random (50% basic, 25% fast, 25% flying), and
every type gains health as the waves go on. Because flyers ignore the maze
completely, a wall of cannons answers only part of the game — a defence with no
anti-air usually falls on the last wave.

## How it fits together

| File | Responsibility |
| --- | --- |
| `main.cpp` | Creates one `Game` and runs it. |
| `Game.h/.cpp` | Match state and the two loops: waves, and ticks within a wave. |
| `Grid.h/.cpp` | The board, the towers on it, and the rules for what may be built where. |
| `Pathfinder.h/.cpp` | Breadth-first search from spawn to base. |
| `Renderer.h/.cpp` | Turns the board and the enemies into the picture above. |
| `Tower.h`, `Enemy.h` | Data-only structs, header-only with no matching `.cpp`. |

## Design notes

**Breadth-first search for routing.** BFS explores outwards in rings from the
spawn, so the first time it reaches the base it has done so in the fewest steps.
Each cell records which cell it was reached from, and the route is rebuilt by
walking those breadcrumbs back from the base. An empty result means the base is
unreachable, which is the signal the build rules depend on.

**The base can never be walled off.** Rather than trying to reason about which
shapes would trap the enemies, `Grid::wouldBlockBase` copies the whole board,
places the tower on the copy, and runs the pathfinder. If the route comes back
empty, the placement is refused and the real board was never touched. One
search reused as a game rule, instead of a special case.

**Flying enemies are one flag, not a second system.** The same BFS runs with an
`ignoreTowers` switch. Ground enemies get the route that treats towers as solid;
flyers get the route that treats them as thin air. Each enemy stores its own
copy, so the rest of the game does not need to know which kind it is holding.

**Speed without a second clock.** Every enemy is ticked at the same rate, but
each carries a `moveDelay` — how many ticks it waits between steps. A fast
enemy has a delay of 1 and moves every tick; a basic enemy waits 3. No timers
and no floating point.

**Enemies are never written into the board.** The grid stays a clean, permanent
record of what has been built. Enemies are drawn as an overlay onto a fresh copy
each frame, which is the same static-world-plus-entities split real renderers
use.

## Possible extensions

- Selling or moving towers, and a refund on sale
- More tower kinds, such as a slowing tower
- Boss enemies on every fifth wave
- Reading wave definitions from a file instead of generating them
- Unit tests around the pathfinder and the build rules
