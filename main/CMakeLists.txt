idf_component_register(
    SRCS
      "CanSatState.cpp"

        "MainAction.cpp"

        "ProGram_STANDBY.cpp"

        "ProGram_FRYING.cpp"

        "ProGram_START.cpp"
        
        "ProGram_RUN.cpp"

        "ProGram_GOAL.cpp"

        "telemetry.cpp"

        "../components.sencer/GPS用コード.cpp"

    INCLUDE_DIRS "."
)