#!/usr/bin/env ruby

if $*.empty?
  puts "\n\t#{$0} [header names...]\n"
  exit 1
end

$*.each do |f|
  fp = f + '.h'
  next if File.file? fp
  fm = f + '_h'
  fi = f.upcase + '_IMPLEMENTATION'
  File.open(fp, "w") do |fh|
    fh.write """
/* #{fp}
 *
 *           _____                     _____                     _____
 *          /\    \                   /\    \                   /\    \
 *         /::\____\                 /::\    \                 /::\____\
 *        /:::/    /                /::::\    \               /:::/    /
 *       /:::/    /                /::::::\    \             /:::/    /
 *      /:::/    /                /:::/\:::\    \           /:::/    /
 *     /:::/____/                /:::/__\:::\    \         /:::/    /
 *    /::::\    \               /::::\   \:::\    \       /:::/    /
 *   /::::::\    \   _____     /::::::\   \:::\    \     /:::/    /
 *  /:::/\:::\    \ /\    \   /:::/\:::\   \:::\    \   /:::/    /
 * /:::/  \:::\    /::\____\ /:::/  \:::\   \:::\____\ /:::/____/
 * \::/    \:::\  /:::/    / \::/    \:::\  /:::/    / \:::\    \
 *  \/____/ \:::\/:::/    /   \/____/ \:::\/:::/    /   \:::\    \
 *           \::::::/    /             \::::::/    /     \:::\    \
 *            \::::/    /               \::::/    /       \:::\    \
 *            /:::/    /                /:::/    /         \:::\    \
 *           /:::/    /                /:::/    /           \:::\    \
 *          /:::/    /                /:::/    /             \:::\    \
 *         /:::/    /                /:::/    /               \:::\____\
 *         \::/    /                 \::/    /                 \::/    /
 *          \/____/                   \/____/                   \/____/
 *
 *
 * Created by Rory B. Bellows on 26/11/2017.
 * Copyright © 2017-2019 George Watson. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * *   Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * *   Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * *   Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL GEORGE WATSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef #{fm}
#define #{fm}
#if defined(__cplusplus)
extern \"C\" {
#endif

#if defined(__cplusplus)
}
#endif
#endif /* #{fm} */

/*                /$$$$$$$$ /$$   /$$ /$$$$$$$
 *               | $$_____/| $$$ | $$| $$__  $$
 *               | $$      | $$$$| $$| $$  \ $$
 *               | $$$$$   | $$ $$ $$| $$  | $$
 *               | $$__/   | $$  $$$$| $$  | $$
 *               | $$      | $$\  $$$| $$  | $$
 *               | $$$$$$$$| $$ \  $$| $$$$$$$/
 *               |________/|__/  \__/|_______/
 *
 *
 *
 *                       /$$$$$$  /$$$$$$$$
 *                      /$$__  $$| $$_____/
 *                     | $$  \ $$| $$
 *                     | $$  | $$| $$$$$
 *                     | $$  | $$| $$__/
 *                     | $$  | $$| $$
 *                     |  $$$$$$/| $$
 *                      \______/ |__/
 *
 *
 *
 *    /$$   /$$ /$$$$$$$$  /$$$$$$  /$$$$$$$  /$$$$$$$$ /$$$$$$$
 *   | $$  | $$| $$_____/ /$$__  $$| $$__  $$| $$_____/| $$__  $$
 *   | $$  | $$| $$      | $$  \ $$| $$  \ $$| $$      | $$  \ $$
 *   | $$$$$$$$| $$$$$   | $$$$$$$$| $$  | $$| $$$$$   | $$$$$$$/
 *   | $$__  $$| $$__/   | $$__  $$| $$  | $$| $$__/   | $$__  $$
 *   | $$  | $$| $$      | $$  | $$| $$  | $$| $$      | $$  \ $$
 *   | $$  | $$| $$$$$$$$| $$  | $$| $$$$$$$/| $$$$$$$$| $$  | $$
 *   |__/  |__/|________/|__/  |__/|_______/ |________/|__/  |__/
 */

#if defined(#{fi})
#endif // #{fi} 

/*              /$$$$$$$$ /$$   /$$ /$$$$$$$
 *             | $$_____/| $$$ | $$| $$__  $$
 *             | $$      | $$$$| $$| $$  \ $$
 *             | $$$$$   | $$ $$ $$| $$  | $$
 *             | $$__/   | $$  $$$$| $$  | $$
 *             | $$      | $$\  $$$| $$  | $$
 *             | $$$$$$$$| $$ \  $$| $$$$$$$/
 *             |________/|__/  \__/|_______/
 *
 *
 *
 *                     /$$$$$$  /$$$$$$$$
 *                    /$$__  $$| $$_____/
 *                   | $$  \ $$| $$
 *                   | $$  | $$| $$$$$
 *                   | $$  | $$| $$__/
 *                   | $$  | $$| $$
 *                   |  $$$$$$/| $$
 *                    \______/ |__/
 *
 *
 *
 *   /$$$$$$   /$$$$$$  /$$   /$$ /$$$$$$$   /$$$$$$  /$$$$$$$$
 *  /$$__  $$ /$$__  $$| $$  | $$| $$__  $$ /$$__  $$| $$_____/
 * | $$  \__/| $$  \ $$| $$  | $$| $$  \ $$| $$  \__/| $$
 * |  $$$$$$ | $$  | $$| $$  | $$| $$$$$$$/| $$      | $$$$$
 *  \____  $$| $$  | $$| $$  | $$| $$__  $$| $$      | $$__/
 *  /$$  \ $$| $$  | $$| $$  | $$| $$  \ $$| $$    $$| $$
 * |  $$$$$$/|  $$$$$$/|  $$$$$$/| $$  | $$|  $$$$$$/| $$$$$$$$
 *  \______/  \______/  \______/ |__/  |__/ \______/ |________/
 */

#if defined(HAL_DEBUG)
#include <stdio.h>

int main(int argc, const char* argv[]) {
  return 0;
}
#endif // HAL_DEBUG
"""
  end
end
