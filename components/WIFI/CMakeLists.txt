set(COMPONENT_ADD_INCLUDEDIRS .)

set(COMPONENT_SRCS "link_wifi.c")

set(COMPONENT_REQUIRES MSG lwip freertos CONFIG)

register_component()
