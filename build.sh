export LD_LIBRARY_PATH=/opt/amdgpu-pro/lib/x86_64-linux-gnu
cd build
cmake ../
make
mv mandelbrotcl ../
