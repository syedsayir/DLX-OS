default:
	cd makeprocs; make
	cd producer; make
	cd consumer; make

clean:
	cd makeprocs; make clean
	cd consumer; make clean
	cd producer; make clean

run:
	cd ../../bin;pwd; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 5;pwd; ee469_fixterminal; pwd
