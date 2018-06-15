#include "../graphics.h"

#define MAP_W 24
#define MAP_H 24

static int map[MAP_W][MAP_H] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

#define MOVE_LEFT  0x0001
#define MOVE_RIGHT 0x0002
#define MOVE_UP    0x0004
#define MOVE_DOWN  0x0008

int main(void) {
  surface_t* win = screen("raycaster", 512, 384);
  if (!win) {
    fprintf(stderr, "%s", get_last_error());
    return 1;
  }
  
  double posX = 22, posY = 12;  //x and y start position
  double dirX = -1, dirY = 0; //initial direction vector
  double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane
  
  const int TICKS_PER_SECOND = 60;
  const int SKIP_TICKS = 1000 / TICKS_PER_SECOND;
  const int MAX_FRAMESKIP = 10;
  long next_game_tick = ticks();
  int loops;
  float interpolation;
  
  user_event_t ue;
  int running = 1;
  short input = 0;
  while (!should_close() && running) {
    loops = 0;
    while(ticks() > next_game_tick && loops < MAX_FRAMESKIP) {
      while (poll_events(&ue)) {
        switch (ue.type) {
          case WINDOW_CLOSED:
            running = 0;
            break;
          case KEYBOARD_KEY_DOWN:
#if defined(__APPLE__)
            if (ue.sym == KB_KEY_Q && ue.mod & KB_MOD_SUPER)
              running = 0;
#endif
            switch (ue.sym) {
              case KB_KEY_UP:
              case KB_KEY_W:
                input |= MOVE_UP;
                break;
              case KB_KEY_DOWN:
              case KB_KEY_S:
                input |= MOVE_DOWN;
                break;
              case KB_KEY_LEFT:
              case KB_KEY_A:
                input |= MOVE_LEFT;
                break;
              case KB_KEY_RIGHT:
              case KB_KEY_D:
                input |= MOVE_RIGHT;
                break;
              default:
                break;
            }
            break;
          case KEYBOARD_KEY_UP:
            switch (ue.sym) {
              case KB_KEY_UP:
              case KB_KEY_W:
                input = input & ~MOVE_UP;
                break;
              case KB_KEY_DOWN:
              case KB_KEY_S:
                input = input & ~MOVE_DOWN;
                break;
              case KB_KEY_LEFT:
              case KB_KEY_A:
                input = input & ~MOVE_LEFT;
                break;
              case KB_KEY_RIGHT:
              case KB_KEY_D:
                input = input & ~MOVE_RIGHT;
                break;
              default:
                break;
            }
          default:
            break;
        }
      }
      
      next_game_tick += SKIP_TICKS;
      loops++;
    }
    
    interpolation = (float)(ticks() + SKIP_TICKS - next_game_tick) / (float)SKIP_TICKS;
    
    fill(win, BLACK);
    
    double moveSpeed = interpolation * .05;
    double rotSpeed = interpolation * .02;
    
    if (input & MOVE_UP) {
      if (map[(int)(posX + dirX * moveSpeed)][(int)posY] == 0)
        posX += dirX * moveSpeed;
      if (map[(int)posX][(int)(posY + dirY * moveSpeed)] == 0)
        posY += dirY * moveSpeed;
    }
    
    if (input & MOVE_DOWN) {
      if (map[(int)(posX - dirX * moveSpeed)][(int)posY] == 0)
        posX -= dirX * moveSpeed;
      if (map[(int)posX][(int)(posY - dirY * moveSpeed)] == 0)
        posY -= dirY * moveSpeed;
    }
    
    if (input & MOVE_LEFT) {
      double oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }
    
    if (input & MOVE_RIGHT) {
      double oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
      double oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
    
    for(int x = 0; x < win->w; x++) {
      double cameraX = 2 * x / (double)win->w - 1; //x-coordinate in camera space
      double rayPosX = posX;
      double rayPosY = posY;
      double rayDirX = dirX + planeX * cameraX;
      double rayDirY = dirY + planeY * cameraX;
      int mapX = (int)rayPosX;
      int mapY = (int)rayPosY;
      
      double sideDistX, sideDistY, perpWallDist;
      double deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
      double deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
      
      int stepX, stepY, hit = 0, side;
      if (rayDirX < 0) {
        stepX = -1;
        sideDistX = (rayPosX - mapX) * deltaDistX;
      }
      else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
      }
      
      if (rayDirY < 0) {
        stepY = -1;
        sideDistY = (rayPosY - mapY) * deltaDistY;
      } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
      }
      
      while (hit == 0) {
        if (sideDistX < sideDistY) {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        } else {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        
        if (map[mapX][mapY] > 0)
          hit = 1;
      }
      
      if (side == 0)
        perpWallDist = (mapX - rayPosX + (1 - stepX) / 2) / rayDirX;
      else
        perpWallDist = (mapY - rayPosY + (1 - stepY) / 2) / rayDirY;
      
      int lineHeight = (int)(win->h / perpWallDist);
      
      int drawStart = -lineHeight / 2 + win->h / 2;
      if (drawStart < 0)
        drawStart = 0;
      int drawEnd = lineHeight / 2 + win->h / 2;
      if (drawEnd >= win->h)
        drawEnd = win->h - 1;
      
      int color;
      switch(map[mapX][mapY]) {
        case 1:  color = RED;    break;
        case 2:  color = LIME;   break;
        case 3:  color = BLUE;   break;
        case 4:  color = PINK;  break;
        default: color = WHITE; break;
      }
      
      if (side == 1)
        color = color / 2;
      
      yline(win, x, drawStart, drawEnd, color);
    }
    
    render();
  }
  
  release();
  return 0;
}
