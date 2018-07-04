#include "../graphics.h"

#define MAP_W 24
#define MAP_H 24

// Adapted from: https://lodev.org/cgtutor/raycasting.html

static int map[MAP_W][MAP_H]= {
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
  {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
  {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
  {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
  {4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
  {6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
  {4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3}
};

#define MOVE_LEFT  0x0001
#define MOVE_RIGHT 0x0002
#define MOVE_UP    0x0004
#define MOVE_DOWN  0x0008

#define SW 512
#define SH 384

int main(void) {
  surface_t win;
  if (!screen("raycaster", SW, SH) || !surface(&win, SW, SH)) {
    fprintf(stderr, "%s", get_last_error());
    return 1;
  }
  
  double posX = 22.0, posY = 11.5;  //x and y start position
  double dirX = -1.0, dirY = 0.0; //initial direction vector
  double planeX = 0.0, planeY = 0.66; //the 2d raycaster version of camera plane
  
  long prev_frame_tick;
  long curr_frame_tick = ticks();
  
  surface_t* images[8];
  for (int i = 0; i < 8; ++i) {
    images[i] = malloc(sizeof(surface_t));
    surface(images[i], 64, 64);
  }
  for(int x = 0; x < 64; x++)
    for(int y = 0; y < 64; y++)
    {
      int xorcolor = (x * 256 / 64) ^ (y * 256 / 64);
      //int xcolor = x * 256 / texWidth;
      int ycolor = y * 256 / 64;
      int xycolor = y * 128 / 64 + x * 128 / 64;
      images[0]->buf[64 * y + x] = 65536 * 254 * (x != y && x != 64 - y); //flat red texture with black cross
      images[1]->buf[64 * y + x] = xycolor + 256 * xycolor + 65536 * xycolor; //sloped greyscale
      images[2]->buf[64 * y + x] = 256 * xycolor + 65536 * xycolor; //sloped yellow gradient
      images[3]->buf[64 * y + x] = xorcolor + 256 * xorcolor + 65536 * xorcolor; //xor greyscale
      images[4]->buf[64 * y + x] = 256 * xorcolor; //xor green
      images[5]->buf[64 * y + x] = 65536 * 192 * (x % 16 && y % 16); //red bricks
      images[6]->buf[64 * y + x] = 65536 * ycolor; //red gradient
      images[7]->buf[64 * y + x] = 128 + 256 * 128 + 65536 * 128; //flat grey texture
    }
  
  user_event_t ue;
  int running = 1;
  short input = 0;
  while (!should_close() && running) {
    prev_frame_tick = curr_frame_tick;
    curr_frame_tick = ticks();
    
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
    
    fill(&win, BLACK);
    
    double speed = (curr_frame_tick - prev_frame_tick) / 10.;
    double moveSpeed = speed * .05;
    double rotSpeed = speed * .025;
    
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
    
    for(int x = 0; x < win.w; x++)
    {
      //calculate ray position and direction
      double cameraX = 2*x/(double)win.w-1; //x-coordinate in camera space
      double rayPosX = posX;
      double rayPosY = posY;
      double rayDirX = dirX + planeX*cameraX;
      double rayDirY = dirY + planeY*cameraX;
      
      //which box of the map we're in
      int mapX = (int)rayPosX;
      int mapY = (int)rayPosY;
      
      //length of ray from current position to next x or y-side
      double sideDistX;
      double sideDistY;
      
      //length of ray from one x or y-side to next x or y-side
      double deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX));
      double deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY));
      double perpWallDist;
      
      //what direction to step in x or y-direction (either +1 or -1)
      int stepX;
      int stepY;
      
      int hit = 0; //was there a wall hit?
      int side; //was a NS or a EW wall hit?
      
      //calculate step and initial sideDist
      if (rayDirX < 0)
      {
        stepX = -1;
        sideDistX = (rayPosX - mapX) * deltaDistX;
      }
      else
      {
        stepX = 1;
        sideDistX = (mapX + 1.0 - rayPosX) * deltaDistX;
      }
      if (rayDirY < 0)
      {
        stepY = -1;
        sideDistY = (rayPosY - mapY) * deltaDistY;
      }
      else
      {
        stepY = 1;
        sideDistY = (mapY + 1.0 - rayPosY) * deltaDistY;
      }
      //perform DDA
      while (hit == 0)
      {
        //jump to next map square, OR in x-direction, OR in y-direction
        if (sideDistX < sideDistY)
        {
          sideDistX += deltaDistX;
          mapX += stepX;
          side = 0;
        }
        else
        {
          sideDistY += deltaDistY;
          mapY += stepY;
          side = 1;
        }
        //Check if ray has hit a wall
        if (map[mapX][mapY] > 0) hit = 1;
      }
      
      //Calculate distance of perpendicular ray (oblique distance will give fisheye effect!)
      if (side == 0) perpWallDist = (mapX - rayPosX + (1 - stepX) / 2) / rayDirX;
      else           perpWallDist = (mapY - rayPosY + (1 - stepY) / 2) / rayDirY;
      
      //Calculate height of line to draw on screen
      int lineHeight = (int)(win.h / perpWallDist);
      
      //calculate lowest and highest pixel to fill in current stripe
      int drawStart = -lineHeight / 2 + win.h / 2;
      if(drawStart < 0) drawStart = 0;
      int drawEnd = lineHeight / 2 + win.h / 2;
      if(drawEnd >= win.h) drawEnd = win.h - 1;
      
      //texturing calculations
      int texNum = map[mapX][mapY] - 1; //1 subtracted from it so that texture 0 can be used!
      
      //calculate value of wallX
      double wallX; //where exactly the wall was hit
      if (side == 0) wallX = rayPosY + perpWallDist * rayDirY;
      else           wallX = rayPosX + perpWallDist * rayDirX;
      wallX -= floor((wallX));
      
      //x coordinate on the texture
      int texX = (int)(wallX * 64.);
      if(side == 0 && rayDirX > 0) texX = 64 - texX - 1;
      if(side == 1 && rayDirY < 0) texX = 64 - texX - 1;
      
      for(int y = drawStart; y < drawEnd; y++)
      {
        int d = y * 256 - win.h * 128 + lineHeight * 128;  //256 and 128 factors to avoid floats
        int texY = ((d * 64) / lineHeight) / 256;
        int color = images[texNum]->buf[64 * texY + texX];
        //make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
        if(side == 1) color = (color >> 1) & 8355711;
        pset(&win, x, y, color);
      }
    }
    
    render(&win);
  }
  
  destroy(&win);
  for (int i = 0; i < 8; ++i) {
    destroy(images[i]);
    free(images[i]);
  }
  release();
  return 0;
}
