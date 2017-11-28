//
//  app.h
//  app
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#ifndef app_h
#define app_h
#ifdef __cplusplus
extern "C" {
#endif
  
int  app_open(const char*, int, int);
int  app_update(void*);
void app_close(void);

#ifdef __cplusplus
}
#endif
#endif /* app_h */
