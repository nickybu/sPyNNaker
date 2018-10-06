from spynnaker.pyNN.models.neuron.neuron_models import NeuronModelIzh
from spynnaker.pyNN.models.neuron.synapse_types import SynapseTypeCombExp4E4I
from spynnaker.pyNN.models.neuron.input_types import InputTypeCurrentPfc
from spynnaker.pyNN.models.neuron.threshold_types import ThresholdTypeStatic
from spynnaker.pyNN.models.neuron import AbstractPyNNNeuronModelStandard
from spynnaker.pyNN.models.defaults import default_initial_values
import numpy

_IZK_THRESHOLD = 30.0
DEFAULT_MAX_ATOMS_PER_CORE = 64

baseline_defaults = {
        'x_a_response': 0,
        'x_a_A': 1,
        'x_a_tau': 50,
        'x_b_response': 0,
        'x_b_B': -1,
        'x_b_tau': 1,

        'i_a_response': 0,
        'i_a_A': 1,
        'i_a_tau': 5,
        'i_b_response': 0,
        'i_b_B': -1,
        'i_b_tau': 10
        }


default_parameters = {

    ##### synapse parameters #####
    # excitatory
    'exc_a_response':baseline_defaults['x_a_response'],
    'exc_a_A':baseline_defaults['x_a_A'],
    'exc_a_tau':baseline_defaults['x_a_tau'],
    'exc_b_response':baseline_defaults['x_b_response'],
    'exc_b_B':baseline_defaults['x_b_B'],
    'exc_b_tau':baseline_defaults['x_b_tau'],

    # excitatory2
    'exc2_a_response':baseline_defaults['x_a_response'],
    'exc2_a_A':baseline_defaults['x_a_A'],
    'exc2_a_tau':baseline_defaults['x_a_tau'],
    'exc2_b_response':baseline_defaults['x_b_response'],
    'exc2_b_B':baseline_defaults['x_b_B'],
    'exc2_b_tau':baseline_defaults['x_b_tau'],

    # excitatory3
    'exc3_a_response':baseline_defaults['x_a_response'],
    'exc3_a_A':baseline_defaults['x_a_A'],
    'exc3_a_tau':baseline_defaults['x_a_tau'],
    'exc3_b_response':baseline_defaults['x_b_response'],
    'exc3_b_B':baseline_defaults['x_b_B'],
    'exc3_b_tau':baseline_defaults['x_b_tau'],

    # excitatory4
    'exc4_a_response':baseline_defaults['x_a_response'],
    'exc4_a_A':baseline_defaults['x_a_A'],
    'exc4_a_tau':baseline_defaults['x_a_tau'],
    'exc4_b_response':baseline_defaults['x_b_response'],
    'exc4_b_B':baseline_defaults['x_b_B'],
    'exc4_b_tau':baseline_defaults['x_b_tau'],

    # inhibitory
    'inh_a_response':baseline_defaults['i_a_response'],
    'inh_a_A':baseline_defaults['i_a_A'],
    'inh_a_tau':baseline_defaults['i_a_tau'],
    'inh_b_response':baseline_defaults['i_b_response'],
    'inh_b_B':baseline_defaults['i_b_B'],
    'inh_b_tau':baseline_defaults['i_b_tau'],

    # inhibitory2
    'inh2_a_response':baseline_defaults['i_a_response'],
    'inh2_a_A':baseline_defaults['i_a_A'],
    'inh2_a_tau':baseline_defaults['i_a_tau'],
    'inh2_b_response':baseline_defaults['i_b_response'],
    'inh2_b_B':baseline_defaults['i_b_B'],
    'inh2_b_tau':baseline_defaults['i_b_tau'],

    # inhibitory3
    'inh3_a_response':baseline_defaults['i_a_response'],
    'inh3_a_A':baseline_defaults['i_a_A'],
    'inh3_a_tau':baseline_defaults['i_a_tau'],
    'inh3_b_response':baseline_defaults['i_b_response'],
    'inh3_b_B':baseline_defaults['i_b_B'],
    'inh3_b_tau':baseline_defaults['i_b_tau'],

    # inhibitory4
    'inh4_a_response':baseline_defaults['i_a_response'],
    'inh4_a_A':baseline_defaults['i_a_A'],
    'inh4_a_tau':baseline_defaults['i_a_tau'],
    'inh4_b_response':baseline_defaults['i_b_response'],
    'inh4_b_B':baseline_defaults['i_b_B'],
    'inh4_b_tau':baseline_defaults['i_b_tau'],


    ##############################
    }



class IzkCurrCombExp4E4I(AbstractPyNNNeuronModelStandard):

    # noinspection PyPep8Naming
    @default_initial_values({"v", "u",
                             "exc_a_response", "exc_b_response",
                             "exc2_a_response", "exc2_b_response",
                             "exc3_a_response", "exc3_b_response",
                             "exc4_a_response", "exc4_b_response",
                             "inh_a_response", "inh_b_response",
                             "inh2_a_response", "inh2_b_response",
                             "inh3_a_response", "inh3_b_response",
                             "inh4_a_response", "inh4_b_response",
                             })

    def __init__(
            self, a=0.02, b=0.2, c=-65.0, d=2.0, i_offset=0.0, u=-14.0,
            v=-70.0,

            # excitatory
            exc_a_response=default_parameters['exc_a_response'],
            exc_a_A=default_parameters['exc_a_A'],
            exc_a_tau=default_parameters['exc_a_tau'],
            exc_b_response=default_parameters['exc_b_response'],
            exc_b_B=default_parameters['exc_b_B'],
            exc_b_tau=default_parameters['exc_b_tau'],

            # excitatory2
            exc2_a_response=default_parameters['exc2_a_response'],
            exc2_a_A=default_parameters['exc2_a_A'],
            exc2_a_tau=default_parameters['exc2_a_tau'],
            exc2_b_response=default_parameters['exc2_b_response'],
            exc2_b_B=default_parameters['exc2_b_B'],
            exc2_b_tau=default_parameters['exc2_b_tau'],

            # excitatory3
            exc3_a_response=default_parameters['exc3_a_response'],
            exc3_a_A=default_parameters['exc3_a_A'],
            exc3_a_tau=default_parameters['exc3_a_tau'],
            exc3_b_response=default_parameters['exc3_b_response'],
            exc3_b_B=default_parameters['exc3_b_B'],
            exc3_b_tau=default_parameters['exc3_b_tau'],

            # excitatory4
            exc4_a_response=default_parameters['exc4_a_response'],
            exc4_a_A=default_parameters['exc4_a_A'],
            exc4_a_tau=default_parameters['exc4_a_tau'],
            exc4_b_response=default_parameters['exc4_b_response'],
            exc4_b_B=default_parameters['exc4_b_B'],
            exc4_b_tau=default_parameters['exc4_b_tau'],

            # inhibitory
            inh_a_response=default_parameters['inh_a_response'],
            inh_a_A=default_parameters['inh_a_A'],
            inh_a_tau=default_parameters['inh_a_tau'],
            inh_b_response=default_parameters['inh_b_response'],
            inh_b_B=default_parameters['inh_b_B'],
            inh_b_tau=default_parameters['inh_b_tau'],

            # inhibitory2
            inh2_a_response=default_parameters['inh2_a_response'],
            inh2_a_A=default_parameters['inh2_a_A'],
            inh2_a_tau=default_parameters['inh2_a_tau'],
            inh2_b_response=default_parameters['inh2_b_response'],
            inh2_b_B=default_parameters['inh2_b_B'],
            inh2_b_tau=default_parameters['inh2_b_tau'],

            # inhibitory3
            inh3_a_response=default_parameters['inh3_a_response'],
            inh3_a_A=default_parameters['inh3_a_A'],
            inh3_a_tau=default_parameters['inh3_a_tau'],
            inh3_b_response=default_parameters['inh3_b_response'],
            inh3_b_B=default_parameters['inh3_b_B'],
            inh3_b_tau=default_parameters['inh3_b_tau'],

            # inhibitory4
            inh4_a_response=default_parameters['inh4_a_response'],
            inh4_a_A=default_parameters['inh4_a_A'],
            inh4_a_tau=default_parameters['inh4_a_tau'],
            inh4_b_response=default_parameters['inh4_b_response'],
            inh4_b_B=default_parameters['inh4_b_B'],
            inh4_b_tau=default_parameters['inh4_b_tau'],
            ):


        # Construct model objects
        neuron_model = NeuronModelIzh(
            a, b, c, d, v, u, i_offset)

        synapse_type = SynapseTypeCombExp4E4I(
                # excitatory
                exc_a_response,
                exc_a_A,
                exc_a_tau,
                exc_b_response,
                exc_b_B,
                exc_b_tau,

                # excitatory2
                exc2_a_response,
                exc2_a_A,
                exc2_a_tau,
                exc2_b_response,
                exc2_b_B,
                exc2_b_tau,

                # excitatory3
                exc3_a_response,
                exc3_a_A,
                exc3_a_tau,
                exc3_b_response,
                exc3_b_B,
                exc3_b_tau,

                # excitatory4
                exc4_a_response,
                exc4_a_A,
                exc4_a_tau,
                exc4_b_response,
                exc4_b_B,
                exc4_b_tau,

                # inhibitory
                inh_a_response,
                inh_a_A,
                inh_a_tau,
                inh_b_response,
                inh_b_B,
                inh_b_tau,

                # inhibitory2
                inh2_a_response,
                inh2_a_A,
                inh2_a_tau,
                inh2_b_response,
                inh2_b_B,
                inh2_b_tau,

                # inhibitory3
                inh3_a_response,
                inh3_a_A,
                inh3_a_tau,
                inh3_b_response,
                inh3_b_B,
                inh3_b_tau,

                # inhibitory4
                inh4_a_response,
                inh4_a_A,
                inh4_a_tau,
                inh4_b_response,
                inh4_b_B,
                inh4_b_tau)

        input_type = InputTypeCurrentPfc()

        threshold_type = ThresholdTypeStatic(_IZK_THRESHOLD)

        super(IzkCurrCombExp4E4I, self).__init__(
            model_name="IZK_curr_comb_exp_4E4I", binary="IZK_curr_comb_exp_4E4I.aplx",
            neuron_model=neuron_model, input_type=input_type,
            synapse_type=synapse_type, threshold_type=threshold_type)



