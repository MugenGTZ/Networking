rm -f ../libnetcrypt.a
g++ -Wall -c netCrypt.c
ar -cvq ../libnetcrypt.a netCrypt.o
echo "FILES->"
ar -t ../libnetcrypt.a
