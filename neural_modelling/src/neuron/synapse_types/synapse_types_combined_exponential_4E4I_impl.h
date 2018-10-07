/*! \file
 * \brief implementation of synapse_types.h for a synapse behaviour
 *  calculated as the difference between two exponential functions
 */

#ifndef _DIFF_SYNAPSE_H_
#define _DIFF_SYNAPSE_H_

#include "../decay.h"
#include <debug.h>

//---------------------------------------
// Macros
//---------------------------------------
#define SYNAPSE_TYPE_BITS 3
#define SYNAPSE_TYPE_COUNT 8
//#define SYNAPSE_INDEX_BITS 5

#define NUM_EXCITATORY_RECEPTORS 4
#define NUM_INHIBITORY_RECEPTORS 4

 //---------------------------------------
 // Synapse parameters
 //---------------------------------------

input_t excitatory_response[NUM_EXCITATORY_RECEPTORS];
input_t inhibitory_response[NUM_INHIBITORY_RECEPTORS];

typedef struct {
 	input_t a_response;
 	input_t a_A;
 	decay_t a_decay;
 	input_t b_response;
 	input_t b_B;
 	decay_t b_decay;
} bi_exp_parm;


 typedef struct synapse_param_t {

	// 4 excitatory bi-exponential synapses
	bi_exp_parm ex1_str;
	bi_exp_parm ex2_str;
	bi_exp_parm ex3_str;
	bi_exp_parm ex4_str;

	// 4 inhibitory bi-exponential synapses
	bi_exp_parm inh1_str;
	bi_exp_parm inh2_str;
	bi_exp_parm inh3_str;
	bi_exp_parm inh4_str;
 } synapse_param_t;

#include "synapse_types.h"

 //! human readable definition for the positions in the input regions for the
 //! different synapse types.
 typedef enum input_buffer_regions {
 	EXCITATORY, EXCITATORY2, EXCITATORY3, EXCITATORY4, INHIBITORY, INHIBITORY2, INHIBITORY3, INHIBITORY4,
 } input_buffer_regions;


 //static inline bi_exp_parm _shape_input(bi_exp_parm bi_exp_params){
 static inline void _shape_input(bi_exp_parm* bi_exp_params){
	 	bi_exp_params->a_response = decay_s1615(
	 			bi_exp_params->a_response,
	 			bi_exp_params->a_decay);

	 	bi_exp_params->b_response = decay_s1615(
	 			bi_exp_params->b_response,
	 			bi_exp_params->b_decay);
 }

 static inline void synapse_types_shape_input(synapse_param_pointer_t parameter){
 	// EXCITATORY
	_shape_input(&parameter->ex1_str);
	_shape_input(&parameter->ex2_str);
	_shape_input(&parameter->ex3_str);
	_shape_input(&parameter->ex4_str);

	// INHIBITORY
	_shape_input(&parameter->inh1_str);
	_shape_input(&parameter->inh2_str);
	_shape_input(&parameter->inh3_str);
	_shape_input(&parameter->inh4_str);
 }

 static inline void _add_input(bi_exp_parm* bi_exp_params, input_t input){

	 bi_exp_params->a_response =  bi_exp_params->a_response + input;
	 bi_exp_params->b_response = bi_exp_params->b_response + input;
 }

 static inline void synapse_types_add_neuron_input(
 		index_t synapse_type_index,
 		synapse_param_pointer_t parameter,
         input_t input){

 	if (synapse_type_index == EXCITATORY) {
 		_add_input(&parameter->ex1_str, input);

 	} else if (synapse_type_index == EXCITATORY2) {
 		_add_input(&parameter->ex2_str, input);

 	} else if (synapse_type_index == EXCITATORY3) {
 		_add_input(&parameter->ex3_str, input);

 	} else if (synapse_type_index == EXCITATORY4) {
 		_add_input(&parameter->ex4_str, input);

 	} else if (synapse_type_index == INHIBITORY) {
 		_add_input(&parameter->inh1_str, input);

 	} else if (synapse_type_index == INHIBITORY2) {
 		_add_input(&parameter->inh2_str, input);

 	} else if (synapse_type_index == INHIBITORY3) {
 		_add_input(&parameter->inh3_str, input);

 	} else if (synapse_type_index == INHIBITORY4) {
 		_add_input(&parameter->inh4_str, input);

 	}
 }

 static inline input_t* synapse_types_get_excitatory_input(
 		synapse_param_pointer_t parameter) {

	 // excitatory
	 int_lk_t temp;

	 temp = bitslk((parameter->ex1_str.a_A * parameter->ex1_str.a_response)
			 + (parameter->ex1_str.b_B * parameter->ex1_str.b_response));

	 if ((temp >> 31) & 0x10000) { // if 17th bit is set
		 excitatory_response[0] = 0xffff;
	 } else {
		 excitatory_response[0] = (parameter->ex1_str.a_A * parameter->ex1_str.a_response)
					 + (parameter->ex1_str.b_B * parameter->ex1_str.b_response);
	 }

	 // excitatory2
	 temp = bitslk((parameter->ex2_str.a_A * parameter->ex2_str.a_response)
			 + (parameter->ex2_str.b_B * parameter->ex2_str.b_response));
	 if ((temp >> 31 & 0x10000)){
		 excitatory_response[1] = 0xffff;
	 } else {
		 excitatory_response[1] = ((parameter->ex2_str.a_A * parameter->ex2_str.a_response)
		 					 + (parameter->ex2_str.b_B * parameter->ex2_str.b_response));
	 }

	 // excitatory3
	 temp = bitslk((parameter->ex3_str.a_A * parameter->ex3_str.a_response)
				 + (parameter->ex3_str.b_B * parameter->ex3_str.b_response));

	 if ((temp >> 31 & 0x10000)){
		 excitatory_response[2] = 0xffff;
	 } else {
		 excitatory_response[2] = ((parameter->ex3_str.a_A * parameter->ex3_str.a_response)
		 					 + (parameter->ex3_str.b_B * parameter->ex3_str.b_response));
	 }


	 // excitatory4
	 temp = bitslk((parameter->ex4_str.a_A * parameter->ex4_str.a_response)
			 + (parameter->ex4_str.b_B * parameter->ex4_str.b_response));

	 if ((temp >> 31 & 0x10000)){
		 excitatory_response[3] = 0xffff;
	 } else {
		 excitatory_response[3] = ((parameter->ex4_str.a_A * parameter->ex4_str.a_response)
		 		 + (parameter->ex4_str.b_B * parameter->ex4_str.b_response));
	 }


	 return &excitatory_response[0];
 }

 static inline input_t* synapse_types_get_inhibitory_input(
 		synapse_param_pointer_t parameter) {
	 synapse_types_print_parameters(parameter);

	 int_lk_t temp;

	 temp = bitslk((parameter->inh1_str.a_A * parameter->inh1_str.a_response)
			 + (parameter->inh1_str.b_B * parameter->inh1_str.b_response));

	 if ((temp >> 31 & 0x10000)){
		 inhibitory_response[0] = 0xffff;
	 } else {
		 inhibitory_response[0] = ((parameter->inh1_str.a_A * parameter->inh1_str.a_response)
				 + (parameter->inh1_str.b_B * parameter->inh1_str.b_response));
	 }

	 // inhibitory2
	 temp = bitslk((parameter->inh2_str.a_A * parameter->inh2_str.a_response)
			 + (parameter->inh2_str.b_B * parameter->inh2_str.b_response));

	 if ((temp >> 31 & 0x10000)){
		 inhibitory_response[1] = 0xffff;
	 } else {
		 inhibitory_response[1] = ((parameter->inh2_str.a_A * parameter->inh2_str.a_response)
		 		 + (parameter->inh2_str.b_B * parameter->inh2_str.b_response));
	 }


	 // inhibitory3
	 temp = bitslk((parameter->inh3_str.a_A * parameter->inh3_str.a_response)
			 + (parameter->inh3_str.b_B * parameter->inh3_str.b_response));
	 if ((temp >> 31) & 0x10000) { // if 17th bit is set

		 inhibitory_response[2] = 0xffff;
//		 log_info("synaptic input buffer wrapped");

	 } else {
		 inhibitory_response[2] = ((parameter->inh3_str.a_A * parameter->inh3_str.a_response)
				 + (parameter->inh3_str.b_B * parameter->inh3_str.b_response));
	 }

	 // inhibitory4
	 temp = bitslk((parameter->inh4_str.a_A * parameter->inh4_str.a_response)
			 + (parameter->inh4_str.b_B * parameter->inh4_str.b_response));

	 if ((temp >> 31 & 0x10000)){
		 inhibitory_response[3] =0xffff;
	 } else {
		 inhibitory_response[3] = ((parameter->inh4_str.a_A * parameter->inh4_str.a_response)
		 					 + (parameter->inh4_str.b_B * parameter->inh4_str.b_response));
	 }



	 return &inhibitory_response[0];
 }

 static inline const char *synapse_types_get_type_char(
 		index_t synapse_type_index) {
 	if (synapse_type_index == EXCITATORY) {
 		return "X";
 	}else if (synapse_type_index == EXCITATORY2) {
 		return "X2";
 	}else if (synapse_type_index == EXCITATORY3) {
 		return "X3";
 	}else if (synapse_type_index == EXCITATORY4) {
 		return "X4";
 	}else if (synapse_type_index == INHIBITORY) {
 		return "I";
 	} else if (synapse_type_index == INHIBITORY2) {
 		return "I2";
 	} else if (synapse_type_index == INHIBITORY3) {
 		return "I3";
 	} else if (synapse_type_index == INHIBITORY4) {
 		return "I4";
 	} else {
 		log_debug("did not recognise synapse type %i", synapse_type_index);
 		return "?";
 	}
 }


 static inline void synapse_types_print_input(
         synapse_param_pointer_t parameter) {
	 use(parameter);
 }

 void _print_receptor_struct(bi_exp_parm bi_exp_param){
	 	io_printf(IO_BUF, "a_response  = %11.4k\n", bi_exp_param.a_response);
	 	io_printf(IO_BUF, "a_A  = %11.4k\n", bi_exp_param.a_A);
	 	io_printf(IO_BUF, "a_decay  = %0.12k\n", bi_exp_param.a_decay);

	 	io_printf(IO_BUF, "b_response  = %11.4k\n", bi_exp_param.b_response);
	 	io_printf(IO_BUF, "b_B = %11.4k\n", bi_exp_param.b_B);
	 	io_printf(IO_BUF, "b_decay = %0.12k\n", bi_exp_param.b_decay);
 }

 static inline void synapse_types_print_parameters(synapse_param_pointer_t parameter) {
	 use(parameter);
	 io_printf(IO_BUF, "Receptor 0\n");
	 _print_receptor_struct(parameter->ex1_str);
	 io_printf(IO_BUF, "Receptor 1\n");
	 _print_receptor_struct(parameter->ex2_str);
	 io_printf(IO_BUF, "Receptor 2\n");
	 _print_receptor_struct(parameter->ex3_str);
	 io_printf(IO_BUF, "Receptor 3\n");
	 _print_receptor_struct(parameter->ex4_str);

	 io_printf(IO_BUF, "Receptor 4\n");
	 _print_receptor_struct(parameter->inh1_str);
	 io_printf(IO_BUF, "Receptor 5\n");
	 _print_receptor_struct(parameter->inh2_str);
	 io_printf(IO_BUF, "Receptor 6\n");
	 _print_receptor_struct(parameter->inh3_str);
	 io_printf(IO_BUF, "Receptor 7\n");
	 _print_receptor_struct(parameter->inh4_str);
	 io_printf(IO_BUF, "****************");

 }





 #endif // _DIFF_SYNAPSE_H_

