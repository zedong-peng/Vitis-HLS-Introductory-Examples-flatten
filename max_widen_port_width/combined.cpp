/*
 * Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
 * Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
 * Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
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
#ifndef EXAMPLE__H_
#define EXAMPLE__H_

#include <cstdlib>
#include <iostream>

constexpr uint64_t N = 64;
constexpr uint64_t MAXWBW = 256;

void example(int a[N], int b[N]);

#endif
using namespace std;

void example(int a[N], int b[N]) {
#pragma HLS INTERFACE m_axi port = a depth = N bundle =                        \
    gmem max_widen_bitwidth = MAXWBW
#pragma HLS INTERFACE m_axi port = b depth = N bundle = gmem
    int buff[N];
    for (size_t i = 0; i < N; ++i) {
        buff[i] = a[i];
        buff[i] = buff[i] + 100;
        b[i] = buff[i];
    }
}
