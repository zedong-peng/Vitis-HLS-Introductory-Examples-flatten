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

#ifndef DUC_H
#define DUC_H

#include <fstream>
#include <iostream>
using namespace std;

const int L_INPUT = 200;
const int L_OUTPUT = 16 * L_INPUT;

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

#ifndef DDS_H_
#define DDS_H_

#include <fstream>
#include <iostream>
using namespace std;

#include "ap_fixed.h"
// Missing include: #include <math.h>

// phase accumulator
typedef ap_fixed<32, 1> incr_t; // s.xxxx, +/- 1 = +/- pi = +fs/2 to -fs/2
typedef ap_fixed<32, 1> acc_t;  // s.xxxx, +/- 1 = +/- pi = +fs/2 to -fs/2

// cos lut address, word size
const int NLUT = 10;      // bitwidth for cos lut address, covers one quadrant
const int LUTSIZE = 1024; // 2^NLUT
typedef ap_uint<NLUT + 2> lut_adr_t; // covers 4 quadrant
typedef ap_uint<NLUT> quad_adr_t;    // covers 1 quadrant

// rounding makes huge difference in the noise floor
typedef ap_fixed<18, 1, AP_RND_CONV, AP_SAT_SYM> lut_word_t;
// typedef ap_fixed<18,1> lut_word_t;
// typedef double lut_word_t;

// fine lut address, word size
const int NFINE = 9;      // bitwidth for fine lut address, covers one quadrant
const int FINESIZE = 512; // 2^NFINE
typedef ap_uint<NLUT + 2> fine_adr_t; // covers 4 quadrant
// typedef ap_fixed<18,1,AP_RND_INF,AP_SAT_SYM> fine_word_t;
// typedef ap_fixed<18,1> fine_word_t;
// typedef ap_fixed<18,-8> fine_word_t;
// typedef ap_fixed<18,-10> fine_word_t;
// typedef double fine_word_t;
typedef ap_fixed<18, -7> fine_word_t;

const double DELTA =
    M_PI / (2 * LUTSIZE *
            FINESIZE); // fine lut resolution, range covers 0 to pi/(2*LUTSIZE)

// DDS output = f(cos lut, fine table)
// typedef ap_fixed<18,1,AP_RND_CONV,AP_SAT_SYM> dds_t;
typedef ap_fixed<16, 1, AP_RND_CONV, AP_SAT_SYM> dds_t;

//______________________________________________________________________________
// N:        number of sample to process
// DDS_T:    cos/sin data type
// OUTPUT_T: output data type
//______________________________________________________________________________

template <int N, typename DDS_T, typename OUTPUT_T> class dds_class {

    // tables
    lut_word_t cos_lut[LUTSIZE];
    fine_word_t fine_lut[FINESIZE];

    // accumulator
    acc_t acc;

    // for dithering
    // ap_uint<1> sr[19];

  public:
    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    dds_class() {
        acc = 0;
        // sr[19] = {0,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1};

        // initialize tables
        init_cos_lut(cos_lut, LUTSIZE);
        init_fine_lut(fine_lut, FINESIZE, DELTA);
    }

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    void mix(DDS_T* dds_cos, DDS_T* dds_sin, OUTPUT_T* dout_i, OUTPUT_T* dout_q,
             OUTPUT_T* dout) {

        // static OUTPUT_T  dout_tmp;
        OUTPUT_T dout_tmp;

    L1:
        for (int i = 0; i < N; i++) {
            dout[i] = dout_tmp;
            dout_tmp = dout_i[i] * dds_cos[i] - dout_q[i] * dds_sin[i];
        }
    }

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    void init_cos_lut(lut_word_t cos_lut[LUTSIZE], const int LUTSIZE) {

        double cos_double;
// ofstream fp_dout ("debug.txt");

// #define FULL
//_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
#ifdef MIDPOINT

        // store single quadrant
        for (int i = 0; i < LUTSIZE; i++) {
            // cos_double = cos(2*M_PI*(0.0+(double)i)/(4*LUTSIZE));
            cos_double = cos(2 * M_PI * (0.5 + (double)i) / (4 * LUTSIZE));
            cos_lut[i] = cos_double;
            //      fp_dout << scientific << cos_double <<endl;
        }

#ifdef FULL
        // store full quadrant
        ofstream fp_ideal("ideal.txt");
        for (int i = 0; i < 4 * LUTSIZE; i++) {
            cos_double = cos(2 * M_PI * (0.5 + (double)i) / (4 * LUTSIZE));
            fp_ideal << scientific << cos_double << endl;
        }
#endif

#else
        //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
        // not the mid point

        // store single quadrant
        for (int i = 0; i < LUTSIZE; i++) {
            cos_double = cos(2 * M_PI * (0.0 + (double)i) / (4 * LUTSIZE));
            cos_lut[i] = cos_double;
            // fp_dout << scientific << cos_double <<endl;
        }

#ifdef FULL
        // store full quadrant
        ofstream fp_ideal("ideal.txt");
        for (int i = 0; i < 4 * LUTSIZE; i++) {
            cos_double = cos(2 * M_PI * (0.0 + (double)i) / (4 * LUTSIZE));
            fp_ideal << scientific << cos_double << endl;
        }
#endif

#endif
    }

#if 0
//_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 
void read_cos_lut( lut_word_t cos_lut[LUTSIZE], const int LUTSIZE ) {

  lut_adr_t i;            // cover full quadrant
  quad_adr_t lsb,adr;     // cover 1/4 quadrant
  ap_uint<2>  msb;        // specify which quadrant
  lut_word_t  lut_word; 

//  ofstream fp_dout ("fullcos.txt");

  for (int k=0;k<4*LUTSIZE;k++) {

    i    = k;
    msb  = i(11,10);
    lsb  = i(9,0);

    if (msb==0) {        // right top
       adr      = lsb;
       lut_word = cos_lut[adr];
    } else if (msb==1) {  // left top 
       if (lsb==0) lut_word = 0;
       else { 
         adr      = -lsb;
         lut_word = -cos_lut[adr];
       }
    } else if (msb==3) {  // right bot
       if (lsb==0) lut_word = 0;
       else { 
         adr      = -lsb;
         lut_word =  cos_lut[adr];
       }
    } else             {  // left bot 
         adr      =  lsb;
         lut_word = -cos_lut[adr];
    }

//    fp_dout << scientific << lut_word << endl;

  }

}

//_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 
void read_sine_lut( lut_word_t cos_lut[LUTSIZE], const int LUTSIZE ) {

  lut_adr_t i;            // cover full quadrant
  quad_adr_t lsb,adr;     // cover 1/4 quadrant
  ap_uint<2>  msb;        // specify which quadrant
  lut_word_t  lut_word; 

//  ofstream fp_dout ("fullsine.txt");

  for (int k=0;k<4*LUTSIZE;k++) {

    i    = k;
    msb  = i(11,10);
    lsb  = i(9,0);

    if (msb==1) {         // left top
       adr      = lsb;
       lut_word = cos_lut[adr];
    } else if (msb==2) {  // left bot 
       if (lsb==0) lut_word = 0;
       else { 
         adr      = -lsb;
         lut_word = -cos_lut[adr];
       }
    } else if (msb==0) {  // right top
       if (lsb==0) lut_word = 0;
       else { 
         adr      = -lsb;
         lut_word =  cos_lut[adr];
       }
    } else             {  // right bot 
         adr      =  lsb;
         lut_word = -cos_lut[adr];
    }

//    fp_dout << scientific << lut_word << endl;

  }

}
#endif

#if 0
//_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 
//dithering
// z^19 + z^18 + z^14 + 1, from mathworks
// x^19 + x^6  + x^5 + x + 1, used here, not sure if it is optimum
//
ap_uint<19> dither() {

ap_uint<1> sin;
 
sin = sr[18] ^ sr[5] ^ sr[4] ^ sr[0];

LOOP_sr: 
for (int i=18; i>0; i--) {
    sr[i] = sr[i-1];
}

sr[0] = sin;

//for (int j=18; j>=0; j--)
    //cout<< setw(2) << sr[j];
//cout << endl;

ap_uint<19> whole;
LOOP_whole: 
    for (int i=0; i<19; i++) whole[i] = sr[i];
    //cout << ":   whole = " << setw(16) << whole << endl;
    //cout << setw(16) << whole << endl;

return whole;

}

#endif

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    void init_fine_lut(fine_word_t fine_lut[FINESIZE], const int FINESIZE,
                       const double delta) {

        // double fine_double;
        double sine_double;
        // ofstream fp_dout ("fine.txt");

        for (int i = 0; i < FINESIZE; i++) {
            // fine_double = cos(delta*(double)i);
            sine_double = sin(delta * (double)i);
            fine_lut[i] = sine_double;
            // fp_dout << scientific << fine_double <<", " << scientific <<
            // sine_double <<endl;
        }
    }

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    void process(incr_t incr, DDS_T* cos_out, DDS_T* sin_out) {

        fine_adr_t fine_adr;
        fine_word_t fine_word;

        lut_adr_t full_adr; // cover full quadrant
        quad_adr_t lsb;     // cover 1/4 quadrant
        quad_adr_t cos_adr, sin_adr;

        ap_uint<2> msb; // specify which quadrant
        lut_word_t cos_lut_word;
        lut_word_t sin_lut_word;

        // phase accumulator
        acc += incr;

        // look up cos/sine table
        full_adr = acc(31, 20);
        msb = full_adr(11, 10);
        lsb = full_adr(9, 0);

        // right top
        if (msb == 0) {
            cos_adr = lsb;
            cos_lut_word = cos_lut[cos_adr];

            if (lsb == 0)
                sin_lut_word = 0;
            else {
                sin_adr = -lsb;
                sin_lut_word = cos_lut[sin_adr];
            }

            // left top
        } else if (msb == 1) {
            if (lsb == 0)
                cos_lut_word = 0;
            else {
                cos_adr = -lsb;
                cos_lut_word = -cos_lut[cos_adr];
            }

            sin_adr = lsb;
            sin_lut_word = cos_lut[sin_adr];

            // right bot
        } else if (msb == 3) {
            if (lsb == 0)
                cos_lut_word = 0;
            else {
                cos_adr = -lsb;
                cos_lut_word = cos_lut[cos_adr];
            }
            sin_adr = lsb;
            sin_lut_word = -cos_lut[sin_adr];

            // left bot
        } else {
            cos_adr = lsb;
            cos_lut_word = -cos_lut[cos_adr];

            if (lsb == 0)
                sin_lut_word = 0;
            else {
                sin_adr = -lsb;
                sin_lut_word = -cos_lut[sin_adr];
            }
        }

        // adjustment w/ fine table
        fine_adr = acc(19, 11);
        fine_word = fine_lut[fine_adr];

        DDS_T cos_dds, sin_dds;
        cos_dds = cos_lut_word - sin_lut_word * fine_word;
        sin_dds = sin_lut_word + cos_lut_word * fine_word;

        *cos_out = cos_dds;
        *sin_out = sin_dds;
    }

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    void process_frame(incr_t incr, DDS_T* cos_out, DDS_T* sin_out) {

        DDS_T dds_cos;
        DDS_T dds_sin;

    L1:
        for (int i = 0; i < N; i++) {
            process(incr, &dds_cos, &dds_sin);
            cos_out[i] = dds_cos;
            sin_out[i] = dds_sin;
        }
    }

}; // dds_class

#endif
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

#ifndef _FIR_H
#define _FIR_H

//#include <fstream>
#include <iostream>
// using namespace std;

//______________________________________________________________________________
// filter precision
// - precisions used by all the FIR class
//______________________________________________________________________________

#define FIXED

#ifdef FIXED
#include <ap_fixed.h>

static const int NFRAC = 14;
typedef ap_fixed<3 + NFRAC, 3, AP_TRN, AP_WRAP> DATA_T;
typedef ap_fixed<4 + NFRAC, 4, AP_TRN, AP_WRAP> DATA2_T;
typedef ap_fixed<2 + NFRAC, 2, AP_RND_CONV, AP_WRAP> COEF_T;
typedef ap_fixed<5 + NFRAC, 5, AP_RND_CONV, AP_WRAP>
    PROD_T; // rounding helps ~20db dc offset
typedef ap_fixed<10 + NFRAC, 10, AP_TRN, AP_WRAP> ACC_T;
#else
typedef float DATA_T;
typedef float DATA2_T;
typedef float COEF_T;
typedef float PROD_T;
typedef float ACC_T;
#endif

//______________________________________________________________________________
// interpolate by 2 FIR class
//
// - make sure the coeff is odd length, so both subfilter will be symmetric
// - parameter:
//    -l_WHOLE:   number of taps of prototype filter (not the decomposed
//    subfilter) -l_INPUT:  number of data samples -II_GOAL:   initiation
//    interval goal
//
//______________________________________________________________________________

template <int l_WHOLE, int l_INPUT, int II_GOAL> class interp2_class {

    // use assert to check the template parameter

    static const int INTERP_FACTOR = 2;
    static const int L_SUB = l_WHOLE / INTERP_FACTOR; // 32
    static const int ODD = l_WHOLE % 2;               // 1
    static const int l_TDL = L_SUB + ODD;             // 33
    static const int l_NONZERO = L_SUB / 2 + ODD;     // 17

    DATA_T sr[l_TDL];
    ACC_T acc0, acc1;
    COEF_T coeff[2][l_NONZERO];

  public:
    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    // MAC engine
    ACC_T MAC_preadd(DATA_T din0, DATA_T din1, COEF_T coef, ACC_T acc) {

        DATA2_T preadd = din0 + din1;
        PROD_T prod = preadd * coef;
        ACC_T sum = prod + acc;

        return sum;
    };

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    void process(DATA_T din, DATA_T* dout0, DATA_T* dout1) {

#pragma HLS INLINE

// using 'factor' instead of 'complete' uses BRAM instead of FF
#pragma HLS array_reshape variable = sr complete
#pragma HLS array_reshape variable = coeff complete dim = 0

        acc0 = 0;
        acc1 = 0;

    LOOP_MAC:
        for (int i = 0; i < l_NONZERO - ODD; i++) {

            // even number of taps, has one more than odd one
            acc0 = MAC_preadd(sr[i], sr[l_TDL - 1 - i], coeff[0][i], acc0);

            // odd number of taps
            acc1 =
                MAC_preadd(sr[i], sr[l_TDL - 1 - ODD - i], coeff[1][i], acc1);
        }

        // center tap
        acc0 = MAC_preadd(sr[l_NONZERO - 1], 0, coeff[0][l_NONZERO - 1], acc0);

    LOOP_SR:
        for (int i = l_TDL - 1; i > 0; i--) {
            sr[i] = sr[i - 1];
        }

        sr[0] = din;

        *dout0 = acc0;
        *dout1 = acc1;
    }

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    void process_frame(DATA_T din[l_INPUT], DATA_T dout[2 * l_INPUT]) {

        DATA_T dout0, dout1;
    L_process_frame:
        for (int i = 0; i < l_INPUT; i++) {
            process(din[i], &dout0, &dout1);
            dout[2 * i] = dout0;
            dout[2 * i + 1] = dout1;
        }
    }

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    // initialize coefficient for polyphase decomposition
    // - test out even length
    void init(const COEF_T coef_in[l_WHOLE]) {

        static const int nofphase = 2;

        bool scale = 1;
        COEF_T gain = scale ? 2 : 1; // only for DUC

        for (int i = 0; i < l_NONZERO - ODD; i++)
            for (int k = 0; k < 2; k++) {

                coeff[k][i] = gain * coef_in[2 * i + k];
                // cout <<"["<<k<<"]"<<"["<<i<<"] = " <<2*i+k <<endl;
            }

        // number of taps is one greater for odd phase filter
        if (ODD) {

            int k = 0;
            int i = l_NONZERO - 1;

            coeff[k][i] = gain * coef_in[2 * i];
            // cout <<"["<<k<<"]"<<"["<<i<<"] = " <<2*i <<"<=== last " << endl;
        }
    }

    // constructor
    interp2_class(const COEF_T cin[l_WHOLE]) { init(cin); }

    interp2_class(void) {}

    // destructor
    ~interp2_class(void) {}

}; // interp2_class

//______________________________________________________________________________
// interpolate by 2 halfband fir class
//
// - parameter:
//    -l_WHOLE:   number of taps
//    -l_INPUT:  number of input data samples
//    -II_GOAL:   initiation interval goal
//
//
//    -l_TDL:     number of stages in tap delay line
//    -l_NONZERO: number of unique coefficients
//______________________________________________________________________________

template <int l_WHOLE, int l_INPUT, int II_GOAL> class interp2_hb_class {

    // use assert to check the template parameter
    //  (l_WHOLE+1) % 4 == 0?

    static const int l_TDL = (l_WHOLE + 1) / 2; // (23+1)/2 = 12
    static const int l_NONZERO = 1 + l_TDL / 2; // 1 + 12/2 = 7

    DATA_T sr[l_TDL];
    ACC_T acc;
    COEF_T coeff[l_NONZERO];

  public:
    //_  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _
    //_
    //_
    // MAC engine
    ACC_T MAC_preadd(DATA_T din0, DATA_T din1, COEF_T coef, ACC_T acc) {

        DATA2_T preadd = din0 + din1;
        PROD_T prod = preadd * coef;
        ACC_T sum = prod + acc;

        return sum;
    };

    //_  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _
    //_
    //_
    // filter
    void process(DATA_T din, DATA_T* dout0, DATA_T* dout1) {

#pragma HLS array_reshape variable = sr complete
#pragma HLS array_reshape variable = coeff complete dim = 0

        acc = 0;
    LOOP_MAC:
        for (int i = 0; i < l_NONZERO - 1; i++) {
            acc = MAC_preadd(sr[i], sr[l_TDL - 1 - i], coeff[i], acc);
        }

    LOOP_SR:
        for (int i = l_TDL - 1; i > 0; i--) {
            sr[i] = sr[i - 1];
        }

        sr[0] = din;

        *dout0 = acc;
        *dout1 = coeff[l_NONZERO - 1] * sr[l_NONZERO - 1];
    }

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_ _ _
    // filter frame
    void process_frame(DATA_T din[l_INPUT], DATA_T dout[2 * l_INPUT]) {

        DATA_T dout0, dout1;

    L_process_frame:
        for (int i = 0; i < l_INPUT; i++) {
            process(din[i], &dout0, &dout1);
            dout[2 * i] = dout0;
            dout[2 * i + 1] = dout1;
        }
    }

    //_  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _
    //_
    //_
    // initialize coefficient for halfband filter
    void init(const COEF_T cin[]) {

        bool scale = 1;
        COEF_T gain = scale ? 2 : 1;

        for (int i = 0; i < l_NONZERO - 1; i++) {
            coeff[i] = gain * cin[2 * i];
        }

        int i = l_NONZERO - 1;
        coeff[i] = gain * cin[2 * i - 1];
    };

    // constructor
    interp2_hb_class(const COEF_T cin[l_WHOLE]) { init(cin); }

    interp2_hb_class(void) {}

    // destructor
    ~interp2_hb_class(void) {}

}; // interp2_hb_class

#endif

//______________________________________________________________________________
// filter spec
// - putting these inside the duc class makes it not compile

// SRRC
static const int Lsrrc_WHOLE = 65;
const COEF_T cin_srrc[Lsrrc_WHOLE] = {
    //      #include "srrc_0db.inc"
    8.538552e-06,  4.669178e-07,  -3.697474e-05, 3.052152e-06,  1.093230e-04,
    -2.416268e-05, -2.637924e-04, 9.271878e-05,  5.516021e-04,  -2.673026e-04,
    -1.035981e-03, 6.447576e-04,  1.781800e-03,  -1.374126e-03, -2.845114e-03,
    2.666081e-03,  4.255749e-03,  -4.808402e-03, -6.004074e-03, 8.189170e-03,
    8.028184e-03,  -1.336089e-02, -1.021252e-02, 2.121003e-02,  1.239342e-02,
    -3.346770e-02, -1.437864e-02, 5.450117e-02,  1.597204e-02,  -1.003560e-01,
    -1.700502e-02, 3.163505e-01,  5.173627e-01,  3.163505e-01,  -1.700502e-02,
    -1.003560e-01, 1.597204e-02,  5.450117e-02,  -1.437864e-02, -3.346770e-02,
    1.239342e-02,  2.121003e-02,  -1.021252e-02, -1.336089e-02, 8.028184e-03,
    8.189170e-03,  -6.004074e-03, -4.808402e-03, 4.255749e-03,  2.666081e-03,
    -2.845114e-03, -1.374126e-03, 1.781800e-03,  6.447576e-04,  -1.035981e-03,
    -2.673026e-04, 5.516021e-04,  9.271878e-05,  -2.637924e-04, -2.416268e-05,
    1.093230e-04,  3.052152e-06,  -3.697474e-05, 4.669178e-07,  8.538552e-06};

// HB1
static const int Lhb1_WHOLE = 23;
const COEF_T cin_hb1[Lhb1_WHOLE] = {
    //      #include "b1_fp2.inc"
    -4.347138e-04, 0.000000e+00, 2.889073e-03, 0.000000e+00,  -1.100753e-02,
    0.000000e+00,  3.178616e-02, 0.000000e+00, -8.312888e-02, 0.000000e+00,
    3.098590e-01,  4.999390e-01, 3.098590e-01, 0.000000e+00,  -8.312888e-02,
    0.000000e+00,  3.178616e-02, 0.000000e+00, -1.100753e-02, 0.000000e+00,
    2.889073e-03,  0.000000e+00, -4.347138e-04};

// HB2
static const int Lhb2_WHOLE = 11;
const COEF_T cin_hb2[Lhb2_WHOLE] = {
    //      #include "b2_fp2.inc"
    7.416439e-03,  0.000000e+00, -5.306043e-02, 0.000000e+00,
    2.956309e-01,  4.999390e-01, 2.956309e-01,  0.000000e+00,
    -5.306043e-02, 0.000000e+00, 7.416439e-03};

// HB3
static const int Lhb3_WHOLE = 7;
const COEF_T cin_hb3[Lhb3_WHOLE] = {
    //      #include "b3_fp2.inc"
    -3.216511e-02, 0.000000e+00, 2.821170e-01, 4.999390e-01,
    2.821170e-01,  0.000000e+00, -3.216511e-02};

static const int II_SRRC = 16;
static const int II_HB1 = 8;
static const int II_HB2 = 4;
static const int II_HB3 = 2;

//______________________________________________________________________________
void duc(DATA_T din_i[L_INPUT], DATA_T din_q[L_INPUT], DATA_T dout[L_OUTPUT],
         incr_t incr);

//______________________________________________________________________________
// multi stage filter
// - 4 stages, SRRC -> HB1-> HB2-> HB3 -> to DAC
//______________________________________________________________________________

template <int l_INPUT> class filterStageClass {

  public:
#if 0
// constructor not compiling
interp2_class<Lsrrc_WHOLE,   l_INPUT,   II_SRRC> srrc(cin_srrc);
interp2_hb_class<Lhb1_WHOLE, 2*l_INPUT, II_HB1>  hb1(cin_hb1);
interp2_hb_class<Lhb2_WHOLE, 4*l_INPUT, II_HB2>  hb2(cin_hb2);
interp2_hb_class<Lhb3_WHOLE, 8*l_INPUT, II_HB3>  hb3(cin_hb3);
#endif

    interp2_class<Lsrrc_WHOLE, l_INPUT, II_SRRC> srrc;
    interp2_hb_class<Lhb1_WHOLE, 2 * l_INPUT, II_HB1> hb1;
    interp2_hb_class<Lhb2_WHOLE, 4 * l_INPUT, II_HB2> hb2;
    interp2_hb_class<Lhb3_WHOLE, 8 * l_INPUT, II_HB3> hb3;

    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_
    //_
    void process(DATA_T din[l_INPUT], DATA_T dout[16 * l_INPUT]) {
#pragma HLS dataflow

        DATA_T srrc_dout[2 * l_INPUT];
        DATA_T hb1_dout[4 * l_INPUT];
        DATA_T hb2_dout[8 * l_INPUT];
        srrc.process_frame(din, srrc_dout);
        hb1.process_frame(srrc_dout, hb1_dout);
        hb2.process_frame(hb1_dout, hb2_dout);
        hb3.process_frame(hb2_dout, dout);
    }

//_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
#if 1
    // constructor
    filterStageClass(void) {
        srrc.init(cin_srrc);
        hb1.init(cin_hb1);
        hb2.init(cin_hb2);
        hb3.init(cin_hb3);
    }

    // destructor
    ~filterStageClass(void) {}
#endif
};

//______________________________________________________________________________
// 2 channel multi-stage filter
//______________________________________________________________________________
// template<int l_INPUT>
template <int l_INPUT, const int l_OUTPUT = 16 * l_INPUT>
class filterStageClassTwoChan {

    filterStageClass<l_INPUT> duc_i;
    filterStageClass<l_INPUT> duc_q;

  public:
    //_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
    //_
    //_
    void process(DATA_T din_i[l_INPUT], DATA_T dout_i[l_OUTPUT],
                 DATA_T din_q[l_INPUT], DATA_T dout_q[l_OUTPUT]) {
#pragma HLS INLINE
        duc_i.process(din_i, dout_i);
        duc_q.process(din_q, dout_q);
    }
};

#endif

#if 1
//______________________________________________________________________________
void duc(DATA_T din_i[L_INPUT], DATA_T din_q[L_INPUT], DATA_T dout[L_OUTPUT],
         incr_t incr) {

#pragma HLS interface axis depth = L_INPUT port = din_i
#pragma HLS interface axis depth = L_INPUT port = din_q
#pragma HLS interface axis depth = L_OUTPUT port = dout

#pragma HLS STABLE variable = incr

#pragma HLS dataflow

    static filterStageClassTwoChan<L_INPUT> f0;
    static dds_class<L_OUTPUT, dds_t, DATA_T> dds_0;

    DATA_T dout_i[L_OUTPUT];
    DATA_T dout_q[L_OUTPUT];
    dds_t dds_cos[L_OUTPUT];
    dds_t dds_sin[L_OUTPUT];

    f0.process(din_i, dout_i, din_q, dout_q);
    dds_0.process_frame(incr, dds_cos, dds_sin);
    dds_0.mix(dds_cos, dds_sin, dout_i, dout_q, dout);
}

#endif
