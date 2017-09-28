default:
	cd newfs; make

clean:
	cd newfs; make clean

run:
	cd ../../os; make; cd ../bin; dlxsim -x os.dlx.obj -a -D F -u newfs.dlx.obj; ee469_fixterminal
