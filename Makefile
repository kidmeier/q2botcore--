.SUFFIXES: .cc .class .java .cxx .C .cpp .o .c .l .y


VPATH = .
CC = gcc
CPP = g++
CC_FLAGS = 
CPP_FLAGS = -g -fPIC

CC_INCLUDES = 
CPP_INCLUDES = -I.

CC_DEFINES = 
CPP_DEFINES = 

LD = g++
LD_FLAGS = -pg
LD_ENTRY = 
SUFFIX = a

libq2botcore_OBJECTS = \
	network.o server.o util.o \
	\
	ammo.o\
	armor.o\
	client.o\
	edict.o\
	entities.o\
	entity.o\
	entityType.o\
	gamestate.o\
	health.o\
	item.o\
	object.o\
	player.o\
	powerup.o\
	projectile.o\
	weapon.o\
	main.o
	
all: libq2botcore.$(SUFFIX)

libq2botcore.$(SUFFIX): $(libq2botcore_OBJECTS)
	$(AR) -crus libq2botcore.$(SUFFIX) $(libq2botcore_OBJECTS)

.c.o:
	$(CC) -c $< $(CC_FLAGS) $(CC_DEFINES) $(CC_INCLUDES)
.cpp.o:
	$(CPP) -c $< $(CPP_FLAGS) $(CPP_DEFINES) $(CPP_INCLUDES)
.C.o:
	$(CPP) -c $< $(CPP_FLAGS) $(CPP_DEFINES) $(CPP_INCLUDES)
.cc.o:
	$(CPP) -c $< $(CPP_FLAGS) $(CPP_DEFINES) $(CPP_INCLUDES)
.cxx.o:
	$(CPP) -c $< $(CPP_FLAGS) $(CPP_DEFINES) $(CPP_INCLUDES)

## dependency generation
%.d : %.c
	@$(CC) -MM $(CC_INCLUDES) $< | sed 's/\($(basename $(notdir $<))\)\.o[ :]*/\1.o $(notdir $@) : /g' > $@

%.d : %.cpp
		@$(CXX) -MM $(CPP_INCLUDES) $< | sed 's/\($(basename $(notdir $<))\)\.o[ :]*/\1.o $(notdir $@) : /g' > $@

clean:
	rm -f *.o *.d
	rm -f libq2botcore.$(SUFFIX)

-include $(libq2botcore_OBJECTS:.o=.d)

