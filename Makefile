all:	my_test

clang-test:
	clang -O3 llist.c heap.c main.c -o heap_test
	./heap_test	

gcc-test:
	gcc -O3 llist.c heap.c main.c -o heap_test
	./heap_test

my_test:
	gcc -g -O3 llist.c heap.c my_test.c -o my_test

clean:
	rm heap_test my_test
