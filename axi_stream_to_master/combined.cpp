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

#include "ap_axi_sdata.h"
#include "ap_int.h"
#include "hls_stream.h"

typedef ap_axiu<64, 0, 0, 0> trans_pkt;

void example(hls::stream<trans_pkt>& inStreamTop, ap_uint<64> outTop[1024]);

// Expects max bandwidth at 64 beats burst (for 64-bit data)
static constexpr int MAX_BURST_LENGTH = 64;
static constexpr int BUFFER_FACTOR = 64;

// Buffer sizes
static constexpr int DATA_DEPTH = MAX_BURST_LENGTH * BUFFER_FACTOR;
static constexpr int COUNT_DEPTH = BUFFER_FACTOR;

struct data {
    ap_int<64> data_filed;
    ap_int<1> last;
};

/// Reads from in_stream and in_counts, Write to out_memory
void streamtoparallelwithburst(hls::stream<data>& in_stream,
                               hls::stream<int>& in_counts,
                               ap_uint<64>* out_memory) {
    data in_val;
    do {
        int count = in_counts.read();
        for (int i = 0; i < count; ++i) {
            in_val = in_stream.read();
            out_memory[i] = in_val.data_filed;
        }
        out_memory += count;
    } while (!in_val.last);
}

void getinstream(hls::stream<trans_pkt>& in_stream,
                 hls::stream<data>& out_stream, hls::stream<int>& out_counts) {
    int count = 0;
    trans_pkt in_val;
    do {
        in_val = in_stream.read();
        data out_val = {in_val.data, in_val.last};
        out_stream.write(out_val);
        count++;
        if (count >= MAX_BURST_LENGTH || in_val.last) {
            out_counts.write(count);
            count = 0;
        }
    } while (!in_val.last);
}

void example(hls::stream<trans_pkt>& inStreamTop, ap_uint<64> outTop[1024]) {
#pragma HLS INTERFACE axis register_mode = both register port = inStreamTop
#pragma HLS INTERFACE m_axi max_write_burst_length = 256 latency = 10 depth =  \
    1024 bundle = gmem0 port = outTop
#pragma HLS INTERFACE s_axilite port = outTop bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

#pragma HLS DATAFLOW

    hls::stream<data, DATA_DEPTH> buf;
    hls::stream<int, COUNT_DEPTH> count;

    getinstream(inStreamTop, buf, count);
    streamtoparallelwithburst(buf, count, outTop);
}
