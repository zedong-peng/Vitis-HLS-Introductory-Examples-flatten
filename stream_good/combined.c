/*
 * Copyright 2022 Xilinx, Inc.
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

#ifndef _POINTER_STREAM_GOOD_H_
#define _POINTER_STREAM_GOOD_H_

// Missing include: #include <stdio.h>

typedef int din_t;
typedef int dout_t;

void pointer_stream_good(volatile dout_t* d_o, volatile din_t* d_i);

#endif

void pointer_stream_good(volatile dout_t *d_o, volatile din_t *d_i) {
  din_t acc = 0;

  acc += *d_i;
  acc += *(d_i + 1);
  *d_o = acc;
  acc += *(d_i + 2);
  acc += *(d_i + 3);
  *(d_o + 1) = acc;
}
