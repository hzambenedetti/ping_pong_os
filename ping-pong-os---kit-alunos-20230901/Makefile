all:
	gcc -o ./out/ppos pingpong-scheduler-srtf.c ppos-core-aux.c libppos_static.a

disk:
	gcc -o ./out/ppos_disk -Wall pinpong-disco1.c ppos_disk.c ppos_core. -lrt 

clean:
	rm ./out/*

run: all
	./out/ppos
	
