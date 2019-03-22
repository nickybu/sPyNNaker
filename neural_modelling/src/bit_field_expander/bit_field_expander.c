#include <bit_field.h>
#include <utils.h>
#include <spin1_api.h>
#include <data_specification.h>
#include <debug.h>
#include <neuron/regions.h>
#include <neuron/population_table/population_table.h>
#include <neuron/direct_synapses.h>
#include <neuron/synapse_row.h>

//! store of key to n atoms map
typedef struct key_to_max_atoms_map {
    uint32_t key;
    uint32_t n_atoms;
} key_to_max_atoms_map;

// does minimum neurons to sort out DTCM and get though the synapse init
#define N_NEURONS 1

// does minimum synapse types to sort out DTCM and get though the synapse init
#define N_SYNAPSE_TYPES 1

// used to store the row from the master pop / synaptic matrix, not going to
// be used in reality.
static address_t row_address;

//! master pop address
static address_t master_pop_base_address;

// synaptic matrix base address
static address_t synaptic_matrix_base_address;

// bitfield base address
static address_t bit_field_base_address;

// direct matrix base address
static address_t direct_matrix_region_base_address;

// bitfield builder data base address
static address_t bit_field_builder_base_address;

// used to store the DTCM based master pop entries. (used during pop table
// init, and reading back synaptic rows)
static address_t direct_synapses_address;

// used to store the max row size for DMA reads (used when extracting a
// synapse row from SDRAM
static uint32_t row_max_n_words;

// storage location for the list of key to max atom maps
static struct keys_to_max_atoms {
    uint32_t size;
    key_to_max_atoms_map *entries;
} keys_to_max_atoms;

//! a fake bitfield holder. used to circumvent the need for a bitfield in the
//! master pop table, which we are trying to generate with the use of the
//! master pop table. chicken vs egg
static bit_field_t *fake_bit_fields;

//! \brief used to hold SDRAM read row
static uint32_t *row_data;

//! \brief reads in the vertex region addresses
static void read_in_addresses(void) {
    // get the data (linked to SDRAM tag 2 and assume the app IDs match)
    address_t core_address = data_specification_get_data_address();

    master_pop_base_address = data_specification_get_region(
            POPULATION_TABLE_REGION, core_address);
    synaptic_matrix_base_address = data_specification_get_region(
            SYNAPTIC_MATRIX_REGION, core_address);
    bit_field_base_address = data_specification_get_region(
            BIT_FIELD_FILTER_REGION, core_address);
    direct_matrix_region_base_address = data_specification_get_region(
            DIRECT_MATRIX_REGION, core_address);
    bit_field_builder_base_address = data_specification_get_region(
            BIT_FIELD_BUILDER, core_address);

    // printer
    log_debug("master_pop_table_base_address = %0x", master_pop_base_address);
    log_debug("synaptic_matrix_base_address = %0x",
            synaptic_matrix_base_address);
    log_debug("bit_field_base_address = %0x", bit_field_base_address);
    log_debug("direct_matrix_region_base_address = %0x",
            direct_matrix_region_base_address);
    log_debug("bit_field_builder = %0x", bit_field_builder_base_address);
}

static void read_in_the_key_to_max_atom_map(void) {
    // allocate DTCM for the key to max atom map
    address_t init_posn = bit_field_builder_base_address;
    keys_to_max_atoms.size = *init_posn++;
    log_debug("n keys to max atom map entries is %d",
            keys_to_max_atoms.size);

    keys_to_max_atoms.entries = spin1_malloc(
            sizeof(key_to_max_atoms_map) * keys_to_max_atoms.size);
    if (keys_to_max_atoms == NULL) {
        log_error("failed to allocate DTCM for the key to max atom map");
        rt_error(RTE_ABORT);
    }

    // put map into DTCM
    for (uint32_t i = 0; i < keys_to_max_atoms.size; i++) {
        spin1_memcpy(&keys_to_max_atoms.entries[i], init_posn,
                sizeof(key_to_max_atoms_map));
        init_posn += sizeof(key_to_max_atoms_map) / sizeof(uint32_t);

        // print
        log_debug("entry %d has key %d and n_atoms of %d",
                i, keys_to_max_atoms.entries[i]->key,
                keys_to_max_atoms.entries[i]->n_atoms);
    }
    log_debug("finished reading in key to max atom map");
}


//! \brief deduces n neurons from the key
//! \param[in] mask: the key to convert to n_neurons
//! \return the number of neurons from the key map based off this key
static inline uint32_t n_neurons_from_key(uint32_t key) {
    for (uint32_t i = 0; i < keys_to_max_atoms.size; i++) {
        key_to_max_atoms_map *entry = &keys_to_max_atoms.entries[i];
        if (entry->key == key) {
            return entry->n_atoms;
        }
    }
    log_error("didn't find the key %d in the map. WTF!", key);
    rt_error(RTE_ABORT);
    return 0;
}

//! \brief creates a fake bitfield where every bit is set to 1.
//! \return bool, which states if the creation of the fake bitfield was
//!               successful or not.
static bool create_fake_bit_field(void) {
    fake_bit_fields = spin1_malloc(
            population_table_length() * sizeof(bit_field_t));
    if (fake_bit_fields == NULL) {
        log_error("failed to allocate DTCM for the fake bitfield holders");
        return false;
    }

    // iterate through the master pop entries
    for (uint32_t i = 0; i < population_table_length(); i++) {
        // determine n_neurons
        uint32_t key = population_table_get_spike_for_index(i);
        uint32_t n_neurons = n_neurons_from_key(key);
        log_debug("entry %d, key = %0x, n_neurons = %d",
                i, key, n_neurons);

        // generate the bitfield for this master pop entry
        uint32_t n_words = get_bit_field_size(n_neurons);

        log_debug("n neurons is %d. n words is %d", n_neurons, n_words);
        fake_bit_fields[i] = bit_field_alloc(n_neurons);
        if (fake_bit_fields[i] == NULL) {
            log_error("could not allocate %d bytes of DTCM for bit field",
                    n_words * sizeof(uint32_t));
            return false;
        }

        // set bitfield elements to 1 and store in fake bitfields.
        set_bit_field(fake_bit_fields[i], n_words);
    }
    log_debug("finished fake bit field");
    return true;
}

static void print_fake_bit_field(void) {
    uint32_t length = population_table_length();
    for (uint32_t i = 0; i < length; i++) {
        log_debug("\n\nfield for index %d", i);

        bit_field_t field = (bit_field_t) fake_bit_fields[i];
        uint32_t key = population_table_get_spike_for_index(i);
        uint32_t n_neurons = n_neurons_from_key(key);

        for (uint32_t j = 0; j < n_neurons; j++) {
            if (bit_field_test(field, j)) {
                log_debug("neuron id %d was set", j);
            } else {
                log_debug("neuron id %d was not set", j);
            }
        }
    }
    log_debug("finished bit field print");
}

//! \brief sets up the master pop table and synaptic matrix for the bit field
//!        processing
//! \return: bool that states if the init was successful or not.
static bool initialise(void) {
    // init the synapses to get direct synapse address
    log_debug("direct synapse init");
    if (!direct_synapses_initialise(
            direct_matrix_region_base_address, &direct_synapses_address)) {
        log_error("failed to init the synapses. failing");
        return false;
    }

    // init the master pop table
    log_debug("pop table init");
    if (!population_table_initialise(
            master_pop_base_address, synaptic_matrix_base_address,
            direct_synapses_address, &row_max_n_words)) {
        log_error("failed to init the master pop table. failing");
        return false;
    }

    log_debug("elements in master pop table is %d \n and max rows is %d",
            population_table_length(), row_max_n_words);

    // read in the correct key to max atom map
    read_in_the_key_to_max_atom_map();

    // set up a fake bitfield so that it always says there's something to read
    if (!create_fake_bit_field()) {
        log_error("failed to create fake bit field");
        return false;
    }

    // print fake bitfield
    //_print_fake_bit_field();

    // set up a SDRAM read for a row
    log_debug("allocating DTCM for row data");
    row_data = spin1_malloc(row_max_n_words * sizeof(uint32_t));
    if (row_data == NULL) {
        log_error("could not allocate DTCM for the row data");
        return false;
    }
    log_debug("finished DTCM for row data");
    // set up the fake connectivity lookup into the master pop table

    population_table_set_connectivity_bit_field(fake_bit_fields);
    log_debug("finished pop table set connectivity lookup");

    return true;
}

//! \brief checks plastic and fixed elements to see if there is a target.
//! \param[in] row: the synaptic row
//! \return bool stating true if there is target, false if no target.
static bool process_synaptic_row(synaptic_row_t row) {
    // get address of plastic region from row
    if (synapse_row_plastic_size(row) > 0) {
        log_debug("plastic row had entries, so can't be pruned");
        return true;
    }

    // Get address of non-plastic region from row
    address_t fixed_region_address = synapse_row_fixed_region(row);
    uint32_t fixed_synapse =
            synapse_row_num_fixed_synapses(fixed_region_address);
    if (fixed_synapse == 0) {
        log_debug("plastic and fixed do not have entries, so can be pruned");
        return false;
    } else {
        log_debug("fixed row has entries, so can't be pruned");
        return true;
    }
}

//! \brief do SDRAM read to get synaptic row
//! \param[in] row_address: the SDRAM address to read
//! \param[in] n_bytes_to_transfer: how many bytes to read to get the
//!                                 synaptic row
//! \return bool which states true if there is target, false if no target.
static bool do_sdram_read_and_test(
        address_t row_address, uint32_t n_bytes_to_transfer) {
    spin1_memcpy(row_data, row_address, n_bytes_to_transfer);
    log_debug("process synaptic row");
    return process_synaptic_row(row_data);
}

typedef struct stored_bitfield_t {
    spike_t key;
    uint32_t n_words;
    uint32_t bitfield[];
} stored_bitfield_t;

//! \brief creates the bitfield for this master pop table and synaptic matrix
//! \param[in] vertex_id: the index in the regions.
//! \return bool that states if it was successful at generating the bitfield
static bool generate_bit_field(void) {
    // write how many entries (thus bitfields) are to be generated into SDRAM

    uint32_t position = 0;
    log_debug("mem cpy for pop length");
    bit_field_base_address[position] = population_table_length();
    log_debug("update position");
    position++;

    // iterate through the master pop entries
    log_debug("starting master pop entry bit field generation");
    for (uint32_t i = 0; i < population_table_length(); i++) {

        // determine keys masks and n_neurons
        spike_t key = population_table_get_spike_for_index(i);
        uint32_t mask = population_table_get_mask_for_entry(i);
        uint32_t n_neurons = n_neurons_from_key(key);

        // generate the bitfield for this master pop entry
        uint32_t n_words = get_bit_field_size(n_neurons);

        log_debug("pop entry %d, key = %d, mask = %0x, n_neurons = %d",
                i, (uint32_t) key, mask, n_neurons);
        bit_field_t bit_field = bit_field_alloc(n_neurons);
        if (bit_field == NULL) {
            log_error("could not allocate DTCM for bit field");
            return false;
        }

        // set the bitfield to 0. so assuming a miss on everything
        clear_bit_field(bit_field, n_words);
        log_debug("cleared bit field");

        // update SDRAM with size of this bitfield
        stored_bitfield_t *stored_bitfield = (stored_bitfield_t *)
                &bit_field_base_address[position];
        stored_bitfield->key = key;
        log_debug("putting master pop key %d at 0x%08x", key, &stored_bitfield->key);
        stored_bitfield->n_words = n_words;
        log_debug("putting n words %d at 0x%08x", n_words, &stored_bitfield->n_words);

        // iterate through neurons and ask for rows from master pop table
        log_debug("searching neuron IDs");
        for (uint32_t j = 0; j < n_neurons; j++) {
            // update key with neuron id
            spike_t new_key = (spike_t) (key + j);
            log_debug("new key for neurons %d is %0x", i, new_key);

            // holder for the bytes to transfer if we need to read SDRAM
            size_t n_bytes_to_transfer;
            if (!population_table_get_first_address(
                    new_key, &row_address, &n_bytes_to_transfer)) {
                log_error("should never get here!!! As this would imply a "
                        "master pop entry which has no master pop entry. "
                        "if this is true for all atoms. Would indicate a "
                        "prunable edge");
                return false;
            }

            log_debug("after got address");
            // This is a direct row to process, so will have 1 target, so
            // no need to go further
            if (n_bytes_to_transfer == 0) {
                log_debug("direct synapse");
                bit_field_set(bit_field, j);
            } else {
                // SDRAM read (faking DMA transfer)
                log_debug("DMA read synapse");
                if (do_sdram_read_and_test(row_address, n_bytes_to_transfer)) {
                    bit_field_set(bit_field, j);
                }
            }
            // if returned false, then the bitfield should be set to 0.
            // Which its by default already set to. so do nothing. so no else.
        }

        // write bitfield to SDRAM
        log_debug("writing bitfield to SDRAM for core use");
        log_debug("writing to address 0x%08x, %d words to write",
                stored_bitfield->bitfield, n_words);
        spin1_memcpy(stored_bitfield->bitfield, bit_field,
                n_words * sizeof(uint32_t));
        position += 2 + n_words;

        // free DTCM of bitfield.
        log_debug("freeing the bitfield DTCM");
        sark_free(bit_field);
    }
    return true;
}

void c_main(void) {
    // set to running state
    sark_cpu_state(CPU_STATE_RUN);

    log_info("starting the bit field expander");

    // read in SDRAM data
    read_in_addresses();

    // generate bit field for each vertex regions
    if (!initialise()) {
        log_error("failed to init the master pop and synaptic matrix");
        rt_error(RTE_ABORT);
    }
    log_info("generating bit field");
    if (!generate_bit_field()) {
        log_error("failed to generate bitfield");
        rt_error(RTE_ABORT);
    }

    log_info("successfully processed the bitfield");
}
