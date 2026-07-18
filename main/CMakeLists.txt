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

        "../components.sencer/九軸センサー専用コード.cpp"

    INCLUDE_DIRS "."
    REQUIRES driver esp_event esp_netif esp_timer esp_wifi nvs_flash
             arduino-esp32 Adafruit_Sensor Adafruit_BNO055 Adafruit_BusIO
)