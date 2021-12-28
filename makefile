SYSTEM     = x86-64_osx
LIBFORMAT  = static_pic

# ---------------------------------------------------------------------
# Compiler options
# ---------------------------------------------------------------------

CCOPT = -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD -stdlib=libstdc++

# ---------------------------------------------------------------------
# Link options and libraries
# ---------------------------------------------------------------------

CCLNFLAGS = -m64 -lm -lpthread -framework CoreFoundation -framework IOKit -stdlib=libstdc++



OPTIONS = -ansi -Wall -Wno-sign-compare $(CCFLAGS)
AUX = ts.cpp

CCFLAGS = $(CCOPT)

all: gfp_test

gfp_test: sched_test.cpp $(AUX)
	g++ $(OPTIONS) -Ofast -o gfp_test sched_test.cpp $(AUX) $(CCLNFLAGS)

clean: 
	rm gfp_test





