idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES "adc_sampler" "connectivity" "dht" "mqtt_connector" "persistence" "power_saving" "time_sync" "shtc3"

)


