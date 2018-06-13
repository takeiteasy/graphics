#include "../graphics.h"
#include <stdbool.h>

#define MAP_W 24
#define MAP_H 24

int map[MAP_W][MAP_H] = {
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

int main(void) {
  surface_t* win = screen("stb_image", 512, 384);
  if (!win) {
    fprintf(stderr, "%s", get_last_error());
    return 1;
  }
  
  double posX = 22, posY = 12;  //x and y start position
  double dirX = -1, dirY = 0; //initial direction vector
  double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane
  
  double time = 0; //time of current frame
  double oldTime = 0; //time of previous frame
  
  user_event_t ue;
  int running = 1;
  while (!should_close() && running) {
    memset(win->buf, 0, win->w * win->h + 1);
    
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
        case 4:  color = WHITE;  break;
        default: color = YELLOW; break;
      }
      
      if (side == 1)
        color = color / 2;
      
      yline(win, x, drawStart, drawEnd, color);
    }
    
    oldTime = time;
    time = (double)ticks();
    double frameTime = (time - oldTime) / 100.0;
    print_f(win, 5, 5, WHITE, "fps: %f", 1. / frameTime);
    
    double moveSpeed = frameTime * 5.0;
    double rotSpeed = frameTime * 3.0;
    while (poll_events(&ue)) {
      switch (ue.type) {
        case WINDOW_CLOSED:
          running = 0;
          break;
        case KEYBOARD_KEY_DOWN:
          switch (ue.sym) {
#if defined(__APPLE__)
            case KB_KEY_Q:
              if (ue.mod & KB_MOD_SUPER)
                running = 0;
              break;
#else
            case KB_KEY_F4
              if (ue.mod & KB_MOD_ALT)
                running = 0;
              break;
#endif
            case KB_KEY_UP:
              if (map[(int)(posX + dirX * moveSpeed)][(int)posY] == false)
                posX += dirX * moveSpeed;
              if (map[(int)posX][(int)(posY + dirY * moveSpeed)] == false)
                posY += dirY * moveSpeed;
              break;
            case KB_KEY_DOWN:
              if (map[(int)(posX - dirX * moveSpeed)][(int)posY] == false)
                posX -= dirX * moveSpeed;
              if (map[(int)posX][(int)(posY - dirY * moveSpeed)] == false)
                posY -= dirY * moveSpeed;
              break;
            case KB_KEY_LEFT: {
                double oldDirX = dirX;
                dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
                dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
                double oldPlaneX = planeX;
                planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
                planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
              }
              break;
            case KB_KEY_RIGHT: {
                double oldDirX = dirX;
                dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
                dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
                double oldPlaneX = planeX;
                planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
                planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
              }
              break;
            default:
              break;
          }
          break;
        default:
          break;
      }
    }
    
    render();
  }
  
  release();
  return 0;
}
