#!/usr/bin/env sh
headerdoc2html -udp include/graphics.h -o ./
if [ -d "graphics_h" ]; then
  rm -rf docs
  mv graphics_h docs
else
  echo "Try again, lol"
fi
