# Test_Adios_op

I will be testing caesar thorugh adios here

how to build with adios on my computer same thing for cmake command to make adios just remove adios dir
cmake .. \
 -DADIOS2_DIR=/home/adios/Programs/ADIOS2/install/lib/cmake/adios2 \
 -Dcaesar_DIR=/home/adios/Programs/CAESAR_C/install/lib/cmake/caesar \
 -DTorch_DIR=/home/adios/.local/lib/python3.11/site-packages/torch/share/cmake/Torch

LD_LIBRARY_PATH=/home/adios/Programs/CAESAR_C/install/lib:/home/adios/local/MGARD_install/lib
