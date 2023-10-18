make clean
make release -j 

mkdir -p bin/EN
mkdir -p bin/EN/lib
mkdir -p bin/EN/include

cp *.h bin/EN/include
cp bin/libEasyNetwork.a bin/EN/lib/libEasyNetwork.a