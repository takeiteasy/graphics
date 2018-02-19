//
//  main.c
//  graphics_osx_gl_test
//
//  Created by Rory B. Bellows on 19/02/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include <stdio.h>

#include "3rdparty/glad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "3rdparty/linalgb.h"
#include "../../../graphics.h"

static const int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;

static SDL_Window* window;
static SDL_GLContext context;

#undef GLAD_DEBUG

#ifdef GLAD_DEBUG
void pre_gl_call(const char *name, void *funcptr, int len_args, ...) {
  printf("Calling: %s (%d arguments)\n", name, len_args);
}
#endif

char* glGetError_str(GLenum err) {
  switch (err) {
    case GL_INVALID_ENUM:                  return "INVALID_ENUM"; break;
    case GL_INVALID_VALUE:                 return "INVALID_VALUE"; break;
    case GL_INVALID_OPERATION:             return "INVALID_OPERATION"; break;
    case GL_STACK_OVERFLOW:                return "STACK_OVERFLOW"; break;
    case GL_STACK_UNDERFLOW:               return "STACK_UNDERFLOW"; break;
    case GL_OUT_OF_MEMORY:                 return "OUT_OF_MEMORY"; break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION"; break;
    default:
      return "Unknown Error";
  }
}

void post_gl_call(const char *name, void *funcptr, int len_args, ...) {
  GLenum err = glad_glGetError();
  if (err != GL_NO_ERROR) {
    fprintf(stderr, "ERROR %d (%s) in %s\n", err, glGetError_str(err), name);
    abort();
  }
}

void cleanup() {
  SDL_DestroyWindow(window);
  SDL_GL_DeleteContext(context);
  printf("Goodbye!\n");
}

void print_shader_log(GLuint s) {
  if (glIsShader(s)) {
    int log_len = 0, max_len = 0;
    glGetShaderiv(s, GL_INFO_LOG_LENGTH, &max_len);
    char* log = malloc(sizeof(char) * max_len);
    
    glGetShaderInfoLog(s, max_len, &log_len, log);
    if (log_len >  0)
      printf("%s\n", log);
    
    free(log);
  }
}

GLuint load_shader(const GLchar* src, GLenum type) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, NULL);
  glCompileShader(s);
  
  GLint res = GL_FALSE;
  glGetShaderiv(s, GL_COMPILE_STATUS, &res);
  if (!res) {
    print_shader_log(s);
    return 0;
  }
  
  return s;
}

GLuint create_shader(const GLchar* vs_src, const GLchar* fs_src) {
  GLuint sp = glCreateProgram();
  GLuint vs = load_shader(vs_src, GL_VERTEX_SHADER);
  GLuint fs = load_shader(fs_src, GL_FRAGMENT_SHADER);
  glAttachShader(sp, vs);
  glAttachShader(sp, fs);
  glLinkProgram(sp);
  glDeleteShader(vs);
  glDeleteShader(fs);
  return sp;
}

int main(int argc, const char * argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Failed to initalize SDL!\n");
    return -1;
  }
  
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  
  window = SDL_CreateWindow(argv[0],
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
  if (!window) {
    fprintf(stderr, "Failed to create SDL window!\n");
    return -1;
  }
  
  context = SDL_GL_CreateContext(window);
  if (!context) {
    fprintf(stderr, "Failed to create OpenGL context!\n");
    return -1;
  }
  
  if (!gladLoadGL()) {
    fprintf(stderr, "Failed to load GLAD!\n");
    return -1;
  }
  
#ifdef GLAD_DEBUG
  glad_set_pre_callback(pre_gl_call);
#endif
  
  glad_set_post_callback(post_gl_call);
  
  printf("Vendor:   %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version:  %s\n", glGetString(GL_VERSION));
  printf("GLSL:     %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  
  const GLfloat vertices[8] = {
    0.f,   0.f,
    250.f, 0.f,
    0.f,   250.f,
    250.f, 250.f
  };
  
  const GLfloat clip[4] = { 250.f, 250.f, 500.f, 500.f };
  
  surface_t* test = bmp("/Users/roryb/Documents/git/graphics/Uncompressed-24.bmp");
  int w = test->w;
  int h = test->h;
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, test->w, test->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)test->buf);
  
  GLfloat left   = clip[0] / w;
  GLfloat right  = clip[2] / w;
  GLfloat top    = clip[1] / h;
  GLfloat bottom = clip[3] / h;
  
  const GLfloat texture_coord[8] = {
    bottom, right,
    top, right,
    bottom, left,
    top, left
  };
  
  const GLuint indices[6] = {
    0, 1, 2,
    2, 1, 3
  };
  
  const GLchar* vs =
  "#version 150\n"
  "in vec2 position;"
  "uniform mat4 mvp;"
  "in vec2 texture_coord;"
  "out vec2 texture_coord_vs;"
  "void main() {"
  "   gl_Position = mvp * vec4(position, 0, 1);"
  "   texture_coord_vs = 1.0 - vec2(texture_coord.s, texture_coord.t);"
  "}";
  
  const GLchar* fs =
  "#version 150\n"
  "in vec2 texture_coord_vs;"
  "out vec4 out_color;"
  "uniform sampler2D texture_sampler;"
  "void main() {"
  "   out_color = texture(texture_sampler, texture_coord_vs);"
  "}";
  
  mat4 proj = mat4_orthographic(0.f, 640.f, 480.f, 0.f, -1.f, 1.f);
  mat4 model = mat4_translation(vec3_new(10, 10, 0));
  mat4 mvp = mat4_mul_mat4(proj, model);
  
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof vertices + sizeof texture_coord, NULL, GL_STATIC_DRAW);
  
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof vertices, vertices);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof vertices, sizeof texture_coord, texture_coord);
  
  GLuint ibo;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
  
  GLuint shader  = create_shader(vs, fs);
  glUseProgram(shader);
  
  GLuint mvp_id = glGetUniformLocation(shader, "mvp");
  glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &mvp.m[0]);
  
  model = mat4_translation(vec3_new(10, 10, 0));
  mvp = mat4_mul_mat4(proj, model);
  glUniformMatrix4fv(mvp_id, 1, GL_FALSE, &mvp.m[0]);
  
  GLint position_attribute = glGetAttribLocation(shader, "position");
  glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(position_attribute);
  
  GLint texture_coord_attribute = glGetAttribLocation(shader, "texture_coord");
  glVertexAttribPointer(texture_coord_attribute, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof vertices);
  glEnableVertexAttribArray(texture_coord_attribute);
  
  SDL_bool running = SDL_TRUE;
  SDL_Event e;
  
  Uint32 now = SDL_GetTicks();
  Uint32 then;
  float  delta;
  
  while (running) {
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_QUIT:
          running = SDL_FALSE;
          break;
      }
    }
    
    then = now;
    now = SDL_GetTicks();
    delta = (float)(now - then) / 1000.0f;
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    SDL_GL_SwapWindow(window);
  }
  return 0;
}
