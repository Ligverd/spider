bin_PROGRAMS = spider


# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
spider_LDFLAGS = $(all_libraries) 
spider_LDFLAGS += -lpthread 

spider_SOURCES = client.cpp main.cpp monmessage.cpp netmes.cpp parser.cpp \
	unipar_pf.cpp
noinst_HEADERS = client.h lib.h main.h monmessage.h netmes.h parser.h type.h \
	unipar.h
