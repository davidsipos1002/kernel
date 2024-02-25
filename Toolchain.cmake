set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER x86_64-elf-gcc)
set(CMAKE_C_COMPILER_WORKS 1)
add_compile_options(-ffreestanding -mno-red-zone -mgeneral-regs-only -Wall -Wextra)
add_link_options(-T ${CMAKE_SOURCE_DIR}/kernel.ld -ffreestanding -nostdlib -lgcc)