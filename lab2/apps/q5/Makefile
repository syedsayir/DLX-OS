default:
	cd krypton; make
	cd oprocess; make
	cd nprocess; make

clean:
	cd krypton; make clean
	cd nprocess; make clean
	cd oprocess; make clean

run:
	cd ../../bin;pwd; dlxsim -x os.dlx.obj -a -u krypton.dlx.obj 4 11 80;pwd; ee469_fixterminal; pwd
