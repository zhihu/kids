all: loggen subscribe echo2kids

subscribe: subscribe.c
	gcc -g -o subscribe subscribe.c -lhiredis

loggen: loggen.c
	gcc -g -o loggen loggen.c -lhiredis

echo2kids: echo2kids.c
	gcc -g -o echo2kids echo2kids.c -lhiredis

clean:
	rm -rf loggen subscribe echo2kids
