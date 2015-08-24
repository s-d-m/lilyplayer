include Makefile.vars

TARGET_DIR := ../bin
TARGET := ${TARGET_DIR}/lilyplayer-gui

LIBS += ${QT_LIBS}
DEFINES += ${QT_DEFINES}
INCLUDES += ${QT_INCLUDES}

FORMS := mainwindow.ui
MOC_FILES := ${FORMS:%.ui=moc_%.cc}
FORMS_HEADERS := ${FORMS:%.ui=ui_%.hh}

SRC :=  main.cc  \
	mainwindow.cc \
	${MOC_FILES}

OBJS := ${SRC:.cc=.o}


ifeq ($(findstring clang,$(CXX)), clang)
  CXX_WARN_FLAGS ?= -Weverything \
		    -Wno-c++98-compat \
		    -Wno-c++98-compat-pedantic \
		    -Wno-c99-extensions \
		    -Wno-padded # remove padding warning
else
# for g++ 4.9
  CXX_WARN_FLAGS ?= \
	-Werror \
	-Wno-pedantic \
	-Wall \
	-Wextra \
	-Wformat=2 \
	-Wnonnull \
	-Winit-self \
	-Wmissing-include-dirs \
	-Wswitch-default \
	-Wswitch-enum \
	-Wuninitialized \
	-fstrict-aliasing \
	-Wstrict-overflow=5 \
	-Warray-bounds \
	-Wfloat-equal \
	-Wundef \
	-Wshadow \
	-Wcast-qual \
	-Wcast-align \
	-Wwrite-strings \
	-Wconversion \
	-Wsign-conversion \
	-Wmissing-declarations \
	-Wpacked \
	-Wredundant-decls \
	-Winvalid-pch \
	-Wlong-long \
	-Wvarargs \
	-Wvla \
	-funsafe-loop-optimizations \
	-Wunsafe-loop-optimizations \
	-Wdouble-promotion \
	-Wsuggest-attribute=pure \
	-Wsuggest-attribute=const \
	-Wsuggest-attribute=noreturn \
	-Wsuggest-attribute=format \
	-Wtrampolines \
	-Wzero-as-null-pointer-constant \
	-Wuseless-cast \
	-Wlogical-op \
	-Wvector-operation-performance \
	-Wabi \
	-Wctor-dtor-privacy \
	-Wnoexcept \
	-Weffc++ \
	-Wstrict-null-sentinel \
	-Wold-style-cast \
	-Woverloaded-virtual \
	-Wsign-promo

#	unfortunately g++ reports padding warnings with lambda's variable capturing
#	-Wpadded \

# for G++ 5.1/5.2
  CXX_WARN_FLAGS += \
	-Wswitch-bool \
	-Wlogical-not-parentheses \
	-Wsizeof-array-argument \
	-Wbool-compare  \
	-Wodr \
	-Wsuggest-final-types \
	-Wsuggest-final-methods

# sanitizers on g++5.2
  SANITIZERS = \
	-fsanitize=address \
	-fsanitize=undefined \
# -fcheck-pointer-bounds -mmpx # pointer checker is not supported with address sanitiser
# -fsanitize=thread #is incompatible with sanitize address
# -fsanitize=leak # leak sanitizer can't be used if either address or thread sanitizer is used

endif

BUILD ?= debug

CXXFLAGS = -std=c++14 -Werror -fPIC ${CXX_WARN_FLAGS} ${SANITIZERS} ${DEFINES}

ifeq ($(BUILD),release)
  CXXFLAGS += -O3 -flto
else
  CXXFLAGS += -O0 -ggdb3
endif



ifeq ($(GPROF),1)
  CXXFLAGS += -pg
endif

SCAN_BUILD = scan-build

all: ${TARGET}

${TARGET}: ${OBJS}
	-mkdir -p ${TARGET_DIR}
	${CXX} ${CXXFLAGS} -o ${TARGET} ${OBJS} ${LIBS}

%.o: %.cc
	${CXX} ${CXXFLAGS} ${INCLUDES} -MD -c -o "$@" "$<"
	 @cp $*.d $*.P; \
	   sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	       -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	   rm -f $*.d

moc_%.cc: %.hh ui_%.hh
	${MOC} ${QT_DEFINES} ${QT_INCLUDES:-isystem=-I} -o "$@" "$<" && \
	sed -i '1i // Avoid warnings on generated file\n#pragma GCC diagnostic error "-w"\n' "$@"

ui_%.hh: %.ui
	${UIC} -o "$@" "$<" && \
	sed -i '1i // Avoid warnings on generated headers\n#pragma GCC system_header\n' "$@"

scan-build:
	${SCAN_BUILD} -analyze-headers --use-c++=/usr/bin/clang++ --status-bugs --keep-going  make -B

clean:
	rm -f ${TARGET} ${OBJS} $(SRC:%.cc=$/%.P) ${MOC_FILES} ${FORMS_HEADERS}

.PHONY: all clean

-include $(SRC:%.cc=$/%.P)
