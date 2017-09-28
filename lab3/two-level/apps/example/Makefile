default:
	cd makeprocs; make
	cd hello_world; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean
	cd procs1; make clean
	cd procs2; make clean
	cd procs3; make clean
	cd program; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 100; ee469_fixterminal
	#cd ../../bin; dlxsim -D m -x os.dlx.obj -a -D m -u makeprocs.dlx.obj 100; ee469_fixterminal
