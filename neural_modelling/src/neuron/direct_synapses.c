#include <debug.h>
#include <spin1_api.h>
#include <common/neuron-typedefs.h>

#define SIZE_OF_SINGLE_FIXED_SYNAPSE 4

static uint32_t single_fixed_synapse[SIZE_OF_SINGLE_FIXED_SYNAPSE];

typedef struct direct_synapse_data_t {
    uint32_t size;
    uint32_t data[];
} direct_synapse_data_t;

bool direct_synapses_initialise(
        void *direct_matrix_address, address_t *direct_synapses_address) {
    const direct_synapse_data_t *matrix = direct_matrix_address;
    // Work out the positions of the direct and indirect synaptic matrices
    // and copy the direct matrix to DTCM
    const uint32_t size = matrix->size;
    log_info("Direct matrix malloc size is %d", size);

    if (size != 0) {
        address_t direct_synapses = spin1_malloc(size);
        if (direct_synapses == NULL) {
            log_error("Not enough memory to allocate direct matrix");
            return false;
        }

        *direct_synapses_address = direct_synapses;
        log_debug("Copying %u bytes of direct synapses to 0x%08x",
                size, direct_synapses);
        spin1_memcpy(direct_synapses, matrix->data, size);
    }

    // Set up for single fixed synapses (data that is consistent per direct row)
    single_fixed_synapse[0] = 0;
    single_fixed_synapse[1] = 1;
    single_fixed_synapse[2] = 0;

    return true;
}

synaptic_row_t direct_synapses_get_direct_synapse(address_t row_address){
    single_fixed_synapse[3] = (uint32_t) row_address[0];
    return (synaptic_row_t) single_fixed_synapse;
}
