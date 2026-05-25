
add_library(oled INTERFACE)

target_sources(oled INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/oled.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/font.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/chicago_12.cpp
)

target_include_directories(oled INTERFACE 
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(oled INTERFACE
    pico_stdlib
    hardware_dma
    hardware_gpio
    hardware_i2c
    hardware_irq
)

add_subdirectory(test)
