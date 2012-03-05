.PHONY: clean all pre

# TODO: make nicer.
# work out which udev we have. 

DEBV=$(shell cat /etc/debian_version)
LDFLAGS_EXTRA=
ifeq ($(DEBV),4.0)
	UDEVPRG=-DUSE_OLD_UDEV
else
	UDEVPRG=
	LDFLAGS_EXTRA+=-ludev
endif

APP=diskmanager
APP_SRC=parse.cpp Disks.cpp RaidDevs.cpp LVM.cpp DiskUtils.cpp CmdApp.cpp StatusNotifier.cpp Utils.cpp $(CMD_SRC)
CMD_SRC=FsTabCmd.cpp CmdDevs.cpp UserUmount.cpp UserMount.cpp CmdMD.cpp CmdLV.cpp DiskCmd.cpp
CXXFLAGS_EXTRA = -g -Wall -Wextra -Wold-style-cast -Woverloaded-virtual -Wsign-promo $(UDEVPRG)  $(shell pkg-config --cflags libeutils)
LDFLAGS_EXTRA+= /lib/libparted.so.2 $(shell pkg-config --libs libparted blkid devmapper sigc++-2.0 libeutils)

OBJS=$(APP_SRC:%.cpp=%.o)
DEPDIR = .deps
%.o : %.cpp
	$(COMPILE.cpp) $(CXXFLAGS_EXTRA) -MT $@ -MD -MP -MF $(DEPDIR)/$*.Tpo -o $@ $<
	mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

-include $(SRCS:%.cpp=.deps/%.Po)

all: pre $(APP)

pre:
	@@if [ ! -d .deps ]; then mkdir .deps; fi

$(APP): $(OBJS)
	$(CXX) $^ $(LDFLAGS) $(LDFLAGS_EXTRA) -o $@

clean:                                                                          
	rm -f *~ $(APP) $(OBJS)
	rm -rf .deps
