cmake_minimum_required (VERSION 3.20)
project ("ACAOS")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable("acaos_lect_3_segment_memory" "lect_3_segment_memory.c")
add_executable("acaos_lect_4_real_memory" "lect_4_real_memory.c")
add_executable("acaos_lect_5_mcb" "lect_5_mcb.c")
add_executable("acaos_lect_5_scheduler" "lect_5_scheduler.c")
add_executable("acaos_lect_6_page_memory" "lect_6_page_memory.c")
add_executable("acaos_lect_7_ipc" "lect_7_ipc.c")
