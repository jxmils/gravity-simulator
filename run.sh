rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
chmod +x gravity_sim
./gravity_sim