set(COMPONENT_ADD_INCLUDEDIRS .)

set(COMPONENT_SRCS "socket_init.c" "udp_task.c")

set(COMPONENT_REQUIRES MSG json MEMORY_POOL)

register_component()
