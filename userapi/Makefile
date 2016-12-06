
#DOOCS	=	/afs/ifh.de/group/pitz/doocs
#SYSTEM	=	i586_rhel50
#SYSTEM	=	amd64_rhel60
#SYSTEM	= sun4x_58
#MY_ROOT = /doocs/develop/kalantar
#RUNDIRECTORY = /export/doocs/server/adcsender

TARGET_NAME=upciedev_test

OS		= $(shell uname -s)
#ifeq ($(OS),Linux)
	CPP = g++
	LINK = g++
	CFLAGS= -O2 -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_BSD -D__USE_GNU -D__USE_UNIX98 -I../
#	LIBS= -O2 -lpthread -lrt -lresolv -lnsl -lADCShm -lADCDma -L$(DOOCS)/$(SYSTEM)/lib
#	CFLAGS= -O2 -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_BSD -D__USE_GNU -D__USE_UNIX98 -I$(DOOCS)/develop/kalantar/$(SYSTEM)/lib/include -I$(DOOCS)/$(SYSTEM)/lib/include
#else
	#CPP = CC
	#LINK = CC
	#CFLAGS= -O2 -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_BSD -D__USE_GNU -D__USE_UNIX98 -I/doocs/lib/include
	#LFLAGS  = -O2
	#LIBS = -O2 -lpthread -lrt -lresolv -lsocket -lnsl -L/doocs/lib -lADCShm -L/doocs/lib -lADCDma
	#LIBS = -O2 -lpthread -lrt -lresolv -lsocket -lnsl
	#LIBS = -lsocket -lnsl -lpthread -lrt -L/doocs/lib -lADCShm -lADCDma -L$(MY_ROOT)/$(SYSTEM)/lib -ladcsendertcp
	#LIBS = -lsocket -lnsl -lpthread -lrt -L/doocs/lib -lADCShm -lADCDma
	#LIBS = -lsocket -lnsl -lpthread -lrt /local/lib/libADCShm.a /local/lib/libADCDma.a
#endif



all: $(TARGET_NAME)


##########################################################################################################
upciedev_test_main.o:
	$(CPP) -c $(CFLAGS) -o upciedev_test_main.o	upciedev_test_main.cpp
	
upciedev_local.o:
	$(CPP) -c $(CFLAGS) -o upciedev_local.o		upciedev_local.cpp
	

$(TARGET_NAME): upciedev_test_main.o upciedev_local.o 
						@echo "*** Linking $(TARGET_NAME)"
						$(LINK) *.o $(LIBS) $(LFLAGS) -o $(TARGET_NAME)
						#@rm -f *.o
						#@cp adcsender $(RUNDIRECTORY)/adcsender

#adcsender:  alog.o mutexdv.o criticalsection.o threaddv.o semaphoredv.o smsender.o smserver.o adcdmacltnew.o  
#	$(CPP) $(LFLAGS) -o adcsender alog.o mutexdv.o criticalsection.o threaddv.o semaphoredv.o smsender.o smserver.o adcdmacltnew.o $(LIBS)
						

# Rule for generating object files from C++ source files
.cpp.o: 
	@echo "*** Compiling $<"
	$(CPP) -c $(CFLAGS) $<
	#$(CPP) -c $(CFLAGS) -o "$@" "$<"
	#$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

clean:	
	@rm -f $(TARGET_NAME) *.o

