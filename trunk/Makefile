# This makefile is to be used with GNU make

OS := `uname -o`
AFFOGATOVERSION := `tr -d \"\\"\\"\" < src/affogato.version`
#XSI_HOME := /rsp/apps/3d/xsi/Linux/XSI_5.11_QFE9
#XSI_BINDIR := $(XSI_HOME)/Application/bin
#XSISDK_ROOT := $(XSI_HOME)/XSISDK
#XSI_INCLUDE := $(XSISDK_ROOT)/include

BOOST := /rsp/lib/boost/
BOOST_VER := 1.32.0-r4

SRCDIR := src/
INCDIR := include/
BINDIR := bin/


SOURCES := \
   affogato.cpp \
   affogatoAttribute.cpp \
   affogatoData.cpp \
   affogatoExecute.cpp \
   affogatoGlobals.cpp \
   affogatoHairData.cpp \
   affogatoHelpers.cpp \
   affogatoJob.cpp \
   affogatoJobEngine.cpp \
   affogatoNode.cpp \
   affogatoNurbCurveData.cpp \
   affogatoNurbMeshData.cpp \
   affogatoParticleData.cpp \
   affogatoPass.cpp \
   affogatoPolyMeshData.cpp \
   affogatoProperties.cpp \
   affogatoRenderer.cpp \
   affogatoRiRenderer.cpp \
   affogatoShader.cpp \
   affogatoSphereData.cpp \
   affogatoTokenValue.cpp \
   affogatoWorker.cpp \
   affogatoXmlRenderer.cpp \
   xmlParser/xmlParser.cpp


# Includes
INCLUDES := \
    -Iinclude \
    -Isrc/xmlParser \
    -I$(XSISDK_ROOT)/include \
    -I$(GELATOHOME)/include \
    -I$(DELIGHT)/include \
    -I$(BOOST)/$(BOOST_VER)/include \
    -I$(XSI_INCLUDE)

#release : EXTRADEFINES = -DNDEBUG
#debug : EXTRADEFINES = -DDEBUG

# Defines
DEFINES := \
	-DDELIGHT \
	-D__XSI_PLUGIN \
	-DRSP \
	-DDEBUG

# Compiler flags
CFLAGS_ := -O2
CFLAGS_optimized := $(CFLAGS_) -march=pentium4 -fomit-frame-pointer
CFLAGS_debug := -g -O0 -gstabs+
CFLAGS := $(INCLUDES) $(CFLAGS_$(option))


LIB.dir := lib/
OBJ.dir := obj/
CPPOBJ  := $(patsubst %.cpp,$(OBJ.dir)%.o,$(filter %.cpp,$(SOURCES)))
OBJ     := $(patsubst %.c,$(OBJ.dir)%.o,$(filter %.c,$(SOURCES)))


ifeq "$(OSTYPE)" "irix"  # SGI -- totally untested

	CXX := $(CC)
	LDFLAGS := \
		-n32 \
		-L$(XSIFTK_ROOT)/export/bin \
		-L$(XSI_HOME)/Application/bin \
		-no_transitive_link \
		-lsicppsdk \
		-lXSIFtk
	DEFINES := \
		$(DEFINES) \
		$(EXTRADEFINES) \
		-D_SGI_MP_SOURCE \
		-DUNIX \
		-DAFFOGATOVERSION=\"$(AFFOGATOVERSION)\" \
		-DRSP

else # Linux

	CXX := g++
	LDFLAGS := \
		-L$(DELIGHT)/lib \
		-L/lib/i686 \
		-L$(XSIFTK_ROOT)/export/bin \
		-L$(XSI_HOME)/Application/bin \
		-L$(XSI_HOME)/Application/mainwin/mw/lib-linux_optimized \
		-L/usr/lib \
		-L$(BOOST)/$(BOOST_VER)/lib \
		-lsicppsdk \
		-lm -ldl -lc \
		-l3delight \
		-Wl,-Bstatic,-Bsymbolic -lboost_filesystem-gcc -Wl,-Bdynamic

	# Add these for Gelato support
	#-L$(GELATOHOME)/lib
	#-lgelato

	DEFINES := \
		$(DEFINES) \
		$(EXTRADEFINES) \
		-DLINUX \
		-DUNIX \
		-DXSI_STATIC_LINK \
		-D_GNU \
		-DAFFOGATOVERSION=\"$(AFFOGATOVERSION)\"

endif # ifeq "$(OSTYPE)" "linux-gnu"


$(OBJ.dir)%.o: $(SRCDIR)%.c
	@echo $@
	-@if [ ! -d "$(OBJ.dir)" ]; then mkdir -p "$(OBJ.dir)"; fi
	$(CC) -m32 -c $(CFLAGS) $(DEFINES) $< -o $@

$(OBJ.dir)%.o: $(SRCDIR)%.cpp
	@echo $@
	@if [ ! -d "$(OBJ.dir)" ]; then mkdir -p "$(OBJ.dir)"; fi
	$(CXX) -m32 -c $(CFLAGS) $(DEFINES) $< -o $@

affogato.so: $(OBJ) $(CPPOBJ)
	@echo ________________________________________________________________________________
	@echo Creating $(BINDIR)affogato-$(AFFOGATOVERSION).so
	@$(CXX) -m32 -shared $(CPPOBJ) $(OBJ) $(DEPLIBS) -o $(BINDIR)affogato-$(AFFOGATOVERSION).so $(LDFLAGS)
	@strip --strip-all $(BINDIR)affogato-$(AFFOGATOVERSION).so

depend:
	@-rm .depend
	makedepend -f- -- $(CFLAGS) -- $(SRCDIR)*.cpp > .depend

all: affogato.so

debug: affogato.so

release: affogato.so


clean:
	@-rm -rf $(OBJ.dir)*.o $(OBJ.dir)xmlParser/*.o $(BINDIR)*.so
