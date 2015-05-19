from enum import Enum


class CSAInstructionType(Enum):

    FULL_MASK = 0
    EMPTY_MASK = 1
    ONE_TO_ONE_MASK = 2
    RANDOM_UNIFORM_MASK = 3
    RANDOM_FIXED_NUMBER_MASK = 4
    RANDOM_FIXED_FAN_IN_MASK = 5
    RANDOM_FIXED_FAN_OUT_MASK = 6
    SUM_OPERATOR = 7
    MULTIPLY_OR_INTERSECTION_OPERATOR = 8
    DIFFERENCE_OPERATOR = 9
    COMPLIMENT_OPERATOR = 10
    RANDOM_OPERATOR = 11
    DISC_METRIC = 12
    GAUSSIAN_METRIC = 13
