//
//  graphics_image.h
//  graphics_osx_test
//
//  Created by Rory B. Bellows on 04/07/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#ifndef graphics_image_h
#define graphics_image_h

#include "graphics.h"

int image(surface_t* out, const char* path, int alpha);
const char* get_last_stb_error(void);

#endif /* graphics_image_h */
