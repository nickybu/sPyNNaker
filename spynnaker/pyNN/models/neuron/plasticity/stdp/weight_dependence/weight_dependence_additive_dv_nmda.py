from data_specification.enums import DataType
from spinn_utilities.overrides import overrides
from .abstract_has_scale_boost import AbstractHasScaleAndBoost
from .abstract_weight_dependence import AbstractWeightDependence


class WeightDependenceAdditiveDvDtNMDA(
        AbstractHasScaleAndBoost, AbstractWeightDependence):
    __slots__ = [
        "_w_max",
        "_w_min",
        "_boost_thresh",
        "_causal"]

    # noinspection PyPep8Naming
    def __init__(self, w_min=0.0, w_max=1.0, boost_thresh=1000.0, causal=True):
        super(WeightDependenceAdditiveDvDtNMDA, self).__init__()
        self._w_min = w_min
        self._w_max = w_max
        self._boost_thresh = boost_thresh
        self._causal = causal


    @property
    def w_min(self):
        return self._w_min

    @property
    def w_max(self):
        return self._w_max
        
    @property
    def boost_thresh(self):
        return self._boost_thresh

    @property
    def causal(self):
        return self._causal

    @overrides(AbstractWeightDependence.is_same_as)
    def is_same_as(self, weight_dependence):
        if not isinstance(weight_dependence, WeightDependenceAdditiveDvDtNMDA):
            return False
        return (
            (self._w_min == weight_dependence.w_min)   and
            (self._w_max == weight_dependence.w_max)   and
            (self._scale == weight_dependence.scale) and 
            (self._boost == weight_dependence.boost) and
            (self._boost_thresh == weight_dependence.boost_thresh)
            # and
            # (self._causal == weight_dependence.causal)
        )

    @property
    def vertex_executable_suffix(self):
        return ""

    @overrides(AbstractWeightDependence.get_parameters_sdram_usage_in_bytes)
    def get_parameters_sdram_usage_in_bytes(
            self, n_synapse_types, n_weight_terms):
        if n_weight_terms != 1:
            raise NotImplementedError(
                "Additive DvDt weight dependence only supports one term")
        else:
            return (6 * 4) * n_synapse_types
            # int32 * 6 params * syn types

    @overrides(AbstractWeightDependence.write_parameters)
    def write_parameters(
            self, spec, machine_time_step, weight_scales, n_weight_terms):
        # Loop through each synapse type's weight scale
        for w in weight_scales:

            print(
                "scale %s\twmin %s\twmax %s\trate %s\tboost %s\tbthresh %s\tcausal %s"%\
                (w, int(round(self._w_min * w)), int(round(self._w_max * w)),
                 int(round(self._scale * self._w_max * w)),
                 int(round(self._boost * self._w_max * w)),
                 int(round(self._boost_thresh * float(1 << 15))),
                 int(self._causal)
                 )
            )
            # Scale the weights
            spec.write_value(
                data=int(round(self._w_min * w)), data_type=DataType.INT32)
            spec.write_value(
                data=int(round(self._w_max * w)), data_type=DataType.INT32)

            # Based on http://data.andrewdavison.info/docs/PyNN/_modules/pyNN
            #                /standardmodels/synapses.html
            # Pre-multiply A+ and A- by Wmax
            spec.write_value(data=int(round(self._scale * self._w_max * w)),
                             data_type=DataType.INT32)

            spec.write_value(data=int(round(self._boost * self._w_max * w)),
                             data_type=DataType.INT32)
            #this has to be compared to a REAL (S16.15)
            spec.write_value(data=int(round(self._boost_thresh * float(1 << 15))),
                             data_type=DataType.INT32)

            spec.write_value(data=int(self._causal),
                             data_type=DataType.INT32)

    @property
    def weight_maximum(self):
        return self._w_max

    @overrides(AbstractWeightDependence.get_parameter_names)
    def get_parameter_names(self):
        return ['w_min', 'w_max', 'scale', 'boost', 'boost_thresh', 'causal']
