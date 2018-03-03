//
//  main.c
//  graphics_osx_gl_test
//
//  Created by Rory B. Bellows on 19/02/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include <stdio.h>
#include "graphics.h"

int main(int argc, const char * argv[]) {
  surface_t* test = screen("test", 640, 480);
  
  user_event_t ue;
  while (!should_close()) {
    while (poll_events(&ue));
    render();
  }
  return 0;
}
