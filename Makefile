linux:
	cd samtools-0.1.18; make -f Makefile.linux
	cd src; make; chmod +x fcat.py
	cd liblinear-1.96; make -f Makefile.linux; chmod 755 train; chmod 755 predict
	cd rt-rank_1.5/cart; make
mac:
        cd samtools-0.1.18; make -f Makefile.mac
        cd src; make; chmod +x fcat.py
        cd liblinear-1.96; make -f Makefile.mac; chmod 755 train; chmod 755 predict
        cd rt-rank_1.5/cart; make mac
