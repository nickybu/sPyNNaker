#ifndef _NEURON_MODEL_IZH_CURR_IMPL_H_
#define _NEURON_MODEL_IZH_CURR_IMPL_H_

#include "neuron_model.h"


typedef struct neuron_t {

    // nominally 'fixed' parameters
    REAL A;
    REAL B;
    REAL C;
    REAL D;

    // Variable-state parameters
    REAL V;
    REAL U;

    // offset current [nA]
    REAL I_offset;

    // current timestep - simple correction for threshold
    REAL this_h;
    
    // prev step voltage
    REAL     V_prev;

    // low-pass-filtered version of voltage
    REAL     V_slow;

    // filtered-voltage change in time
    REAL     dV_dt_slow;

    // low-pass weight
    REAL     gamma;
    REAL     gamma_complement;

    REAL     V2_membrane;

    REAL     V_max;

    REAL     V2_threshold;

    int32_t    V2_count;

} neuron_t;

typedef struct global_neuron_params_t {
    REAL machine_timestep_ms;
} global_neuron_params_t;

#endif   // _NEURON_MODEL_IZH_CURR_IMPL_H_
