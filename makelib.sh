cd src
rm -f ../network.a
make rebuild
ar -cvq ../network.a network.o UDP.o TCP.o main.o rand64.o
echo "FILES->"
ar -t ../network.a
cd ..
