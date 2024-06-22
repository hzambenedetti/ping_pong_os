all:
	gcc -o ./out/ppos pingpong-scheduler-srtf.c ppos-core-aux.c libppos_static.a

disk:
	gcc -o ./out/ppos_disk -Wall pingpong-disco1.c ppos_disk.c disk.c ppos-core-aux.c libppos_static.a -lrt

clean:
	rm ./out/*

run: all
	./out/ppos
	
