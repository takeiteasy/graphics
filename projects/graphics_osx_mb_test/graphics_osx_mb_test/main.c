//
//  main.c
//  graphics_osx_mb_test
//
//  Created by Rory B. Bellows on 19/02/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include "my_basic.h"
#include "../../../graphics.h"
#include <assert.h>

static struct mb_interpreter_t* bas = NULL;
static surface_t* window = NULL;

static int __screen(struct mb_interpreter_t* s, void** l) {
  mb_assert(s && l);
  mb_check(mb_attempt_open_bracket(s, l));
  
  int w = 0, h = 0;
  if (mb_has_arg(s, l))
    mb_check(mb_pop_int(s, l, &w));
  if (mb_has_arg(s, l))
    mb_check(mb_pop_int(s, l, &h));
  
  window = screen("quick_basic", w, h);
  if (!window)
    return MB_FUNC_ERR;
  
  mb_check(mb_attempt_close_bracket(s, l));
  
  return MB_FUNC_OK;
}

static int __clear(struct mb_interpreter_t* s, void** l) {
  mb_assert(s);
  if (!window)
    return MB_FUNC_ERR;
  
  fill(window, BLACK);
  
  return MB_FUNC_OK;
}

static int __render(struct mb_interpreter_t* s, void** l) {
  mb_assert(s);
  if (!window)
    return MB_FUNC_ERR;
  
  user_event_t ue;
  while (poll_events(&ue)) {
    
  }
  render();
  
  return MB_FUNC_OK;
}

void cleanup() {
  if (window)
    destroy(&window);
  mb_close(&bas);
  mb_dispose();
}

int main(int argc, const char * argv[]) {
  mb_init();
  atexit(cleanup);
  mb_open(&bas);
  mb_register_func(bas, "SCREEN", __screen);
  mb_register_func(bas, "CLS", __clear);
  mb_register_func(bas, "RENDER", __render);
  mb_load_file(bas, "/Users/roryb/Documents/git/graphics/projects/graphics_osx_mb_test/graphics_osx_mb_test/test.bas");
  mb_run(bas, 1);
  return 0;
}
