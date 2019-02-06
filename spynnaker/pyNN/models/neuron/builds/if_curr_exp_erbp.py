from spynnaker.pyNN.models.neuron import AbstractPyNNNeuronModelStandard
from spynnaker.pyNN.models.defaults import default_initial_values
from spynnaker.pyNN.models.neuron.neuron_models \
    import NeuronModelLeakyIntegrateAndFireERBP
from spynnaker.pyNN.models.neuron.synapse_types import SynapseTypeERBP
from spynnaker.pyNN.models.neuron.input_types import InputTypeCurrent
from spynnaker.pyNN.models.neuron.threshold_types import ThresholdTypeStatic


class IFCurrExpERBP(AbstractPyNNNeuronModelStandard):
    """ Leaky integrate and fire neuron with an exponentially decaying \
        current input, and error compartment
    """

    @default_initial_values({"v", "isyn_exc", "isyn_exc2", "isyn_inh", "isyn_inh2", "local_err"})
    def __init__(
            self, tau_m=20.0, cm=1.0, v_rest=-65.0, v_reset=-65.0,
            v_thresh=-50.0, tau_syn_E=5.0, tau_syn_E2=5.0, tau_syn_I=5.0,
            tau_syn_I2=5.0,
            tau_refrac=0.1,
            i_offset=0.0, v=-65.0,
            isyn_exc=0.0, isyn_exc2=0.0, isyn_inh=0.0, isyn_inh2=0.0,
            local_err=0, tau_err=20):
        # pylint: disable=too-many-arguments, too-many-locals

        neuron_model = NeuronModelLeakyIntegrateAndFireERBP(
            v, v_rest, tau_m, cm, i_offset, v_reset, tau_refrac, local_err, tau_err)

        synapse_type = SynapseTypeERBP(
            tau_syn_E, tau_syn_E2, tau_syn_I, tau_syn_I2, isyn_exc, isyn_exc2, isyn_inh, isyn_inh2)

        input_type = InputTypeCurrent()


        threshold_type = ThresholdTypeStatic(v_thresh)

        super(IFCurrExpERBP, self).__init__(
            model_name="LIF_current_erbp", binary="LIF_current_erbp.aplx",
            neuron_model=neuron_model, input_type=input_type,
            synapse_type=synapse_type, threshold_type=threshold_type)