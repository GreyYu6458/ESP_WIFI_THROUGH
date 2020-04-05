set(COMPONENT_ADD_INCLUDEDIRS .)

set(COMPONENT_SRCS "uart_task.c" "uart_init.c")

set(COMPONENT_REQUIRES MSG MEMORY_POOL)

register_component()
