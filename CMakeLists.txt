# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# arduino-esp32コンポーネント同梱のLittleFS.cppやTLS未使用時の#warningが
# このESP-IDFバージョンでは-Werrorでビルド停止するため、該当警告のみエラー扱いを解除する
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=missing-field-initializers" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=cpp" APPEND)

project(CanSat_2026)
