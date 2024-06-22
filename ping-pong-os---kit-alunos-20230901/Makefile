all:
	gcc -o ./out/ppos pingpong-scheduler-srtf.c ppos-core-aux.c libppos_static.a

disk_one:
	gcc -o ./out/ppos_dis_one -Wall pingpong-disco1.c ppos_disk.c disk.c ppos-core-aux.c libppos_static.a -lrt

disk_two:
	gcc -o ./out/ppos_disk_two -Wall pingpong-disco2.c ppos_disk.c disk.c ppos-core-aux.c libppos_static.a -lrt

clean:
	rm ./out/*

run: all
	./out/ppos
	
