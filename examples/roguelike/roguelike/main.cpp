//
//  main.c
//  roguelike
//
//  Created by Rory B. Bellows on 22/06/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include "game_engine.hpp"

/* TODO:
 * Fix cave room connection
 * A*
 * Player control
 * Camera movement
 * Enemies
 * Background level loading
 */

int main(int argc, const char* argv[]) {
  return game_engine_t(argc, argv).run();
}
