make clean
make release -j%NUMBER_OF_PROCESSORS%

if Not Exist bin\EN mkdir bin\EN
if Not Exist bin\EN\lib mkdir bin\EN\lib
if Not Exist bin\EN\include mkdir bin\EN\include

cp *.h bin/EN/include
cp bin/libEasyNetwork.a bin/EN/lib/libEasyNetwork.a