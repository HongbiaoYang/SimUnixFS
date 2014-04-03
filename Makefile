all: simfs
simfs:simfs.c
	gcc simfs.c -g -o simfs
clean:
	rm -rf *.o simfs
test:
	./simfs
