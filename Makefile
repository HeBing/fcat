all:
	cd samtools-0.1.18; make
	cd src; make
	cd liblinear-1.96; make; chmod 755 train; chmod 755 predict
	cd rt-rank_1.5/cart; make
