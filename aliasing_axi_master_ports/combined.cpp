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

#include <hls_stream.h>
#include <hls_math.h>

const unsigned SIZE = 1024;

template <typename T>
struct dat_t {
	T data;
};

typedef dat_t<uint32_t> A_t;
typedef dat_t<uint32_t> B_t;

const unsigned RATIO  = (sizeof(A_t)/sizeof(B_t));
const unsigned DEPTHA = SIZE/RATIO;
const unsigned DEPTHB = SIZE;

void dut(hls::stream<A_t> &inputStream, A_t *aximA, B_t *aximB,
         hls::stream<B_t> &outputStream);
// Missing include: #include "etc/ap_utils.h"

void write(hls::stream<A_t> &inputStream, A_t *aximA,
           hls::stream<bool> &done) {
#pragma HLS INLINE OFF

    for (unsigned i=0; i<SIZE/RATIO; i++) {
        aximA[i] = inputStream.read();
    }
    
    // Required to separate the write to aximA and write to done. 
    ap_wait();
    
    // This stream acts like a barrier to prevent the read process from
    // starting
    done.write(true);
}

void read(B_t *aximB, hls::stream<B_t> &outputStream,
          hls::stream<bool> &done)
{
#pragma HLS INLINE OFF 
           
    done.read();
    
    ap_wait(); // Required to separate the read of done and read of aximB
           
    for (unsigned i=0; i<SIZE; i++) {
        A_t tmp = aximB[i];
        outputStream << tmp;
    }
}

void dut(hls::stream<A_t> &inputStream, A_t *aximA, B_t *aximB,
         hls::stream<B_t> &outputStream) {
#pragma HLS INTERFACE axis  port=inputStream
#pragma HLS INTERFACE m_axi port=aximA depth=DEPTHA latency=8 offset=direct max_widen_bitwidth=128 max_read_burst_length=256 max_write_burst_length=256 bundle=axim1
#pragma HLS INTERFACE m_axi port=aximB depth=DEPTHB latency=8 offset=direct max_widen_bitwidth=128 max_read_burst_length=256 max_write_burst_length=256 bundle=axim2
#pragma HLS INTERFACE axis  port=outputStream

// Needed to support overlap calls to dut
//#pragma HLS STABLE variable=aximA 
//#pragma HLS STABLE variable=aximB
           
// The ALIAS pragma lets you treat aximA and aximB as pointers to the same memory location. 
#pragma HLS ALIAS ports=aximA,aximB distance=0

#pragma HLS DATAFLOW
    hls::stream<bool> done;

    write(inputStream, aximA, done);
    read(aximB, outputStream, done);
}
