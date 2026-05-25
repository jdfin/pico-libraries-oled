
add_compile_options(-Wall -Wextra -Werror)

add_executable(oled_demo
    oled_demo.cpp
)

pico_enable_stdio_uart(oled_demo 0)
pico_enable_stdio_usb(oled_demo 1)

target_link_libraries(oled_demo PRIVATE
    pico_stdlib
    pico_stdio_usb
    misc
    oled
)

pico_add_extra_outputs(oled_demo)
