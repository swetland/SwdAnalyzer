
CFLAGS := -O3 -Wall -fpic -I. -I../include -DPLUGIN
CXXFLAGS := $(CFLAGS)

OBJS := SwdAnalyzer.o
OBJS += SwdAnalyzerSettings.o
OBJS += SwdAnalyzerResults.o
OBJS += decode-swd.o

all: libSwdAnalyzer.so decode-swd

libSwdAnalyzer.so: $(OBJS)
	g++ -L../lib -lAnalyzer64 -shared -o $@ $(OBJS)

decode-swd: decode-swd.c
	gcc -O2 -Wall -o $@ $<

clean:
	rm -f $(OBJS) libSwdAnalyzer.so decode-swd
