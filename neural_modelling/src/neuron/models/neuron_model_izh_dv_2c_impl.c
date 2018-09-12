#include "neuron_model_izh_dv_2c_impl.h"

#include <debug.h>

//#define LOG_VOLTAGES 1

static global_neuron_params_pointer_t global_params;

/*! \brief For linear membrane voltages, 1.5 is the correct value. However
 * with actual membrane voltage behaviour and tested over an wide range of
 * use cases 1.85 gives slightly better spike timings.
 */
static const REAL SIMPLE_TQ_OFFSET = REAL_CONST(1.85);

/////////////////////////////////////////////////////////////
// definition for Izhikevich neuron
/*static inline void _neuron_ode(
        REAL t, REAL stateVar[], REAL dstateVar_dt[],
        neuron_pointer_t neuron, REAL input_this_timestep) {

    REAL V_now = stateVar[1];
    REAL U_now = stateVar[2];
    log_debug(" sv1 %9.4k  V %9.4k --- sv2 %9.4k  U %9.4k\n", stateVar[1],
              neuron->V, stateVar[2], neuron->U);

    // Update V
    dstateVar_dt[1] =
        REAL_CONST(140.0)
        + (REAL_CONST(5.0) + REAL_CONST(0.0400) * V_now) * V_now - U_now
        + input_this_timestep;

    // Update U
    dstateVar_dt[2] = neuron->A * (neuron->B * V_now - U_now);
} */

/*!
 * \brief Midpoint is best balance between speed and accuracy so far from
 * ODE solve comparison work paper shows that Trapezoid version gives better
 * accuracy at small speed cost
 * \param[in] h
 * \param[in] neuron
 * \param[in] input_this_timestep
 */
static inline void _rk2_kernel_midpoint(REAL h, neuron_pointer_t neuron,
                                        REAL input_this_timestep) {

    // to match Mathematica names
    REAL lastV1 = neuron->V;
    REAL lastU1 = neuron->U;
    REAL a = neuron->A;
    REAL b = neuron->B;

    REAL pre_alph = REAL_CONST(140.000000) + input_this_timestep - lastU1;
    REAL alpha = pre_alph
         + ( REAL_CONST(5.000000) + REAL_CONST(0.040000) * lastV1) * lastV1;
    REAL eta = lastV1 + REAL_HALF(h * alpha);

    // could be represented as a long fract?
    REAL beta = REAL_HALF(h * (b * lastV1 - lastU1) * a);

    neuron->V += h * (pre_alph - beta
          + ( REAL_CONST(5.000000) + REAL_CONST(0.040000) * eta) * eta);

    neuron->U += a * h * (-lastU1 - beta + b * eta);
}

//static inline void _rk2_kernel_midpoint(REAL h, neuron_pointer_t neuron,
//                                        REAL input_this_timestep) {
//    REAL v2 = neuron->V*neuron->V;
//    REAL input = REAL_CONST(140.000000) - neuron->U + input_this_timestep;
//    neuron->V += REAL_CONST(0.500000)*(
//                    REAL_CONST(0.040000)*v2 + \
//                    REAL_CONST(5.000000)*neuron->V + \
//                    input);
//
//    v2 = neuron->V*neuron->V;
//    neuron->V += REAL_CONST(0.500000)*(
//                    REAL_CONST(0.040000)*v2 + \
//                    REAL_CONST(5.000000)*neuron->V + \
//                    input);
//    neuron->U += neuron->A * (neuron->B*neuron->V - neuron->U);
//}

void update_dv_dt(neuron_pointer_t neuron){

    //low-pass filter membrane voltage
    neuron->V_slow = ((neuron->V_slow)*(neuron->gamma)) + \
                      ((neuron->V)*(neuron->gamma_complement));

    // filtered-voltage change this step
    neuron->dV_dt_slow = neuron->V_slow - neuron->V_prev;

    //store previous filtered value
    neuron->V_prev = neuron->V_slow;

}



void neuron_model_set_global_neuron_params(
        global_neuron_params_pointer_t params) {
    global_params = params;
}

state_t neuron_model_state_update(
        uint16_t num_excitatory_inputs, input_t* exc_input,
        uint16_t num_inhibitory_inputs, input_t* inh_input,
        input_t external_bias, neuron_pointer_t neuron) {

    REAL total_exc = 0;
    REAL total_inh = 0;


    total_exc = exc_input[0];
    total_inh = inh_input[0];

    input_t input_this_timestep = total_exc - total_inh
                                  + external_bias + neuron->I_offset;

    // todo: this should be done in an event-based manner, with a LUT
    neuron->V2_membrane = exc_input[1] - inh_input[1];

    neuron->V2_count -= 1;

    if((neuron->V >= neuron->V2_threshold) || neuron->V2_count > 0){

        input_this_timestep += neuron->V2_membrane;
        neuron->V2_count = 5; //todo: make this settable from Python

#if defined(LOG_VOLTAGES)
        log_info("V2 above threshold!!! %11.6k > %11.6k",
            neuron->V2_membrane, neuron->V2_threshold);
#endif
//        input_this_timestep += (neuron->V2_membrane - neuron->V2_threshold);
    }

    // the best AR update so far
    _rk2_kernel_midpoint(neuron->this_h, neuron, input_this_timestep);
    neuron->this_h = global_params->machine_timestep_ms;
    if (neuron->V > neuron->V_max){
        neuron->V = neuron->V_max;
    }

    update_dv_dt(neuron);

#if defined(LOG_VOLTAGES)
     log_info("V, Vs, dVs, V2, I = %11.6k, %11.6k, %11.6k, %11.6k, %11.6k",
         neuron->V, neuron->V_slow, neuron->dV_dt_slow,
         neuron->V2_membrane, input_this_timestep );
#endif

    return neuron->V;
}

void neuron_model_has_spiked(neuron_pointer_t neuron) {

    // reset membrane voltage
    neuron->V = neuron->C;

    // offset 2nd state variable
    neuron->U += neuron->D;

    // simple threshold correction - next timestep (only) gets a bump
    neuron->this_h = global_params->machine_timestep_ms * SIMPLE_TQ_OFFSET;
}

state_t neuron_model_get_membrane_voltage(neuron_pointer_t neuron) {
    return neuron->V;
}


void neuron_model_print_state_variables(restrict neuron_pointer_t neuron) {
    log_debug("V = %11.4k ", neuron->V);
    log_debug("U = %11.4k ", neuron->U);
}

void neuron_model_print_parameters(restrict neuron_pointer_t neuron) {
    log_info("A = %11.4k ", neuron->A);
    log_info("B = %11.4k ", neuron->B);
    log_info("C = %11.4k ", neuron->C);
    log_info("D = %11.4k ", neuron->D);
    log_info("V = %11.4k ", neuron->V);
    log_info("U = %11.4k ", neuron->U);

    log_info("I = %11.4k ", neuron->I_offset);
    
    log_info("V_prev = %11.4k ", neuron->V_prev);
    log_info("V_slow = %11.4k ", neuron->V_slow);
    log_info("dvS = %11.4k ", neuron->dV_dt_slow);
    log_info("G = %11.4k ", neuron->gamma);
    log_info("1-G = %11.4k ", neuron->gamma_complement);
    log_info("V2 = %11.4k", neuron->V2_membrane);
    log_info("V_max = %11.4k", neuron->V_max);
    log_info("V2_threshold = %11.4k\n", neuron->V2_threshold);
    
}
