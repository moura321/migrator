sudo apt-get install libboost-all-dev;
git clone https://gem5.googlesource.com/public/gem5;
cd gem5/;
scons build/X86/gem5.opt;
build/X86/gem5.opt configs/example/se.py -c tests/test-progs/hello/bin/x86/linux/hello;