make libabc.a
gcc -Wall -g -c src/demo.c -o src/demo.o
g++ -g -o cada1027_beta src/demo.o libabc.a -lm -ldl -lreadline -lpthread