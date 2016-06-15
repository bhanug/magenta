// Copyright 2016 The Fuchsia Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gfx/gfx.h>
#include <font/font.h>
#include <stdint.h>

#include "font.h"

#if SMALL_FONT
void font_draw_char(gfx_surface *surface, unsigned char c,
                    int x, int y, uint32_t color, uint32_t bgcolor) {
    uint i,j;
    uint line;

    // draw this char into a buffer
    for (i = 0; i < FONT_Y; i++) {
        line = FONT[c * (FONT_Y * 2) + (i * 2)];
        for (j = 0; j < FONT_X; j++) {
            gfx_putpixel(surface, x + j, y + i, (line & 1) ? color : bgcolor);
            line = line >> 2;
        }
    }
}
#else
void font_draw_char(gfx_surface *surface, unsigned char c,
                    int x, int y, uint32_t color, uint32_t bgcolor) {
    uint i,j;
    uint line;

    // draw this char into a buffer
    for (i = 0; i < FONT_Y; i++) {
        line = FONT[c * FONT_Y + i];
        for (j = 0; j < FONT_X; j++) {
            gfx_putpixel(surface, x + j, y + i, (line & 1) ? color : bgcolor);
            line = line >> 1;
        }
    }
}
#endif
