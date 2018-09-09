//
//  main.c
//  roguelike
//
//  Created by Rory B. Bellows on 22/06/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include "game_engine.hpp"

/* TODO:
 * - Replace heap & stb_sb
 * - Implement no diag pathfinding in A*
 * - Mouse movement
 * - Better way to store tile map
 */

int main(int argc, const char* argv[]) {
  return game_engine_t(argc, argv).run();
}
