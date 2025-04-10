
open_project -reset project
set_top pointer_cast_native
add_files combined.c

open_solution -reset "solution1"
set_part xcu280-fsvh2892-2L-e
create_clock -period 10
csynth_design
close_solution
exit
