CFLAGS = -O2

TIMESTAMP=tml-`date +%Y%m%d`

all:
	mkdir -p ../lib ../bin && \
	gcc $(CFLAGS) -c win_iconv.c && \
	ar crv ../lib/libiconv.a win_iconv.o && \
	gcc $(CFLAGS) -shared -o ../bin/iconv.dll win_iconv_dll.c && \
	rm -f /tmp/win_iconv-$(TIMESTAMP).zip && \
	cp Makefile win_iconv.mk && \
	cd .. && \
	zip /tmp/win_iconv-$(TIMESTAMP).zip README.win_iconv.txt lib/libiconv.a include/iconv.h src/win_iconv.c src/win_iconv.mk && \
	rm -f /tmp/win_iconv_dll-$(TIMESTAMP).zip && \
	zip /tmp/win_iconv_dll-$(TIMESTAMP).zip README.win_iconv.txt bin/iconv.dll src/win_iconv.c src/win_iconv_dll.c src/win_iconv.mk
