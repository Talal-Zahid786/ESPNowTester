#if defined(USE_UNITS_C) && defined(USE_UNITS_F)
#error "Do not define both USE_UNITS_C and USE_UNITS_F. Define only one."
#endif

#if !defined(USE_UNITS_C) && !defined(USE_UNITS_F)
#error "Define either USE_UNITS_C or USE_UNITS_F"
#endif

#if (SENSORS_USED < 1)
#error "SENSORS_USED should be greater than 0"
#endif

#if (SENSORS_USED > 16)
#error "SENSORS_USED should be less than 17"
#endif