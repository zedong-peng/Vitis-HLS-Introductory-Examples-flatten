/*
 * Copyright 2022 AMD, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Copyright 2022 AMD, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAMMING_WINDOW_H_
#define HAMMING_WINDOW_H_

// Missing include: #include <math.h>
// Missing include: #include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// If WINDOW_LEN is not defined by the user, define a default value
#ifndef WINDOW_LEN
#define WINDOW_LEN 256
#endif
// If WIN_COEFF_FRACBITS not defined by user, set a default value
#ifndef WIN_COEFF_FRACBITS
#define WIN_COEFF_FRACBITS (14)
#endif
// Set the scale factor for the window coefficients
#define WIN_COEFF_SCALE ((float)(1 << WIN_COEFF_FRACBITS))

// This function applies an Hamming window function to the 'indata' buffer,
// returning the windowed data in 'outdata'.  The coefficients are 16-bit
// scaled integer, which may be interpreted as a signed fixed point format,
// with WIN_COEFF_FRACBITS bits after the binary point.
void hamming_window(int32_t outdata[], int16_t indata[]);

#endif // HAMMING_WINDOW_H_ not defined

// Translation module function prototypes:
static void hamming_rom_init(int16_t rom_array[]);

// Function definitions:
void hamming_window(int32_t outdata[WINDOW_LEN], int16_t indata[WINDOW_LEN]) {
  static int16_t window_coeff[WINDOW_LEN];
  unsigned i;

  // In order to ensure that 'window_coeff' is inferred and properly
  // initialized as a ROM, it is recommended that the array initialization
  // be done in a sub-function with global (wrt this source file) scope.
  hamming_rom_init(window_coeff);
process_frame:
  for (i = 0; i < WINDOW_LEN; i++) {
    outdata[i] = (int32_t)window_coeff[i] * (int32_t)indata[i];
  }
}

// This initialization function will be optimized away during high level
// sythesis (HLS), resulting in the underlying memory being inferred as a ROM
// by RTL synthesis.
static void hamming_rom_init(int16_t rom_array[WINDOW_LEN]) {
  int i;
  for (i = 0; i < WINDOW_LEN; i++) {
    float real_val =
        0.54f - 0.46f * cos(2.0f * M_PI * i / (float)(WINDOW_LEN - 1));
    rom_array[i] = (int16_t)(round(WIN_COEFF_SCALE * real_val));
  }
}
