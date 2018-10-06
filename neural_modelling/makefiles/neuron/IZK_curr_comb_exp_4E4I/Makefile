APP = $(notdir $(CURDIR))

NEURON_IMPL_H = $(NEURON_DIR)/neuron/implementations/neuron_impl_standard.h
NEURON_MODEL = $(NEURON_DIR)/neuron/models/neuron_model_izh_impl.c
NEURON_MODEL_H = $(NEURON_DIR)/neuron/models/neuron_model_izh_impl.h
INPUT_TYPE_H = $(NEURON_DIR)/neuron/input_types/input_type_current_pfc.h
THRESHOLD_TYPE_H = $(NEURON_DIR)/neuron/threshold_types/threshold_type_static.h
SYNAPSE_TYPE_H = $(NEURON_DIR)/neuron/synapse_types/synapse_types_combined_exponential_4E4I_impl.h
SYNAPSE_DYNAMICS = $(NEURON_DIR)/neuron/plasticity/synapse_dynamics_static_impl.c

include ../neural_build.mk
