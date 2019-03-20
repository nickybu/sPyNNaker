/**
 *! \file
 *! \brief Uniformly distributed random set to boundary parameter generator
 *!        implementation
 */
#include <stdfix.h>
#include <spin1_api.h>
#include <stdfix-full-iso.h>
#include <synapse_expander/rng.h>

/**
 *! \brief The parameters that can be copied in from SDRAM
 */
struct param_generator_uniform_params {
    accum low;
    accum high;
};

/**
 *! \brief The data structure to be passed around for this generator.  This
 *!        includes the parameters and an RNG.
 */
struct param_generator_uniform {
    struct param_generator_uniform_params params;
    rng_t rng;
};

static void *param_generator_uniform_initialize(address_t *region) {

    // Allocate memory for the data
    struct param_generator_uniform *params =
            spin1_malloc(sizeof(struct param_generator_uniform));

    // Copy the parameters in
    spin1_memcpy(
            &params->params, *region,
            sizeof(struct param_generator_uniform_params));
    *region += sizeof(struct param_generator_uniform_params) / sizeof(uint32_t);
    log_debug("Uniform low = %k, high = %k",
            params->params.low, params->params.high);

    // Initialise the RNG for this generator
    params->rng = rng_init(region);
    return params;
}

static void param_generator_uniform_free(void *data) {
    sark_free(data);
}

static void param_generator_uniform_generate(
        void *data, uint32_t n_synapses, uint32_t pre_neuron_index,
        uint16_t *indices, accum *values) {
    use(pre_neuron_index);
    use(indices);

    // For each index, generate a uniformly distributed value
    struct param_generator_uniform *state = data;
    accum range = state->params.high - state->params.low;
    for (uint32_t i = 0; i < n_synapses; i++) {
        values[i] = state->params.low +
                (ulrbits(rng_generator(state->rng)) * range);
    }
}
