idf_component_register(SRCS "mySerial.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES bt driver myBT myFuncs)
