dummy := $(shell if (! test -f ./Makefile.vars) ; then ./configure ; fi)
include Makefile.vars

TARGET_DIR := ../bin
TARGET := ${TARGET_DIR}/lilyplayer

LIBS += -lrtmidi

LIBS += ${QT_LIBS}
DEFINES += ${QT_DEFINES}
INCLUDES += ${QT_INCLUDES}

FORMS := mainwindow.ui
MOC_FILES := ${FORMS:%.ui=moc_%.cc}
FORMS_HEADERS := ${FORMS:%.ui=ui_%.hh}
RC_FILE := ../qdarkstyle/style.qrc
QT_STYLE_FILES := ../qdarkstyle/rc/Vsepartoolbar.png \
		../qdarkstyle/rc/close-pressed.png \
		../qdarkstyle/rc/Hmovetoolbar.png \
		../qdarkstyle/rc/up_arrow.png \
		../qdarkstyle/rc/left_arrow_disabled.png \
		../qdarkstyle/rc/close-hover.png \
		../qdarkstyle/rc/checkbox_unchecked.png \
		../qdarkstyle/rc/radio_unchecked_disabled.png \
		../qdarkstyle/rc/radio_checked.png \
		../qdarkstyle/rc/checkbox_checked_disabled.png \
		../qdarkstyle/rc/Hsepartoolbar.png \
		../qdarkstyle/rc/sizegrip.png \
		../qdarkstyle/rc/down_arrow.png \
		../qdarkstyle/rc/stylesheet-branch-more.png \
		../qdarkstyle/rc/radio_unchecked.png \
		../qdarkstyle/rc/branch_closed-on.png \
		../qdarkstyle/rc/checkbox_unchecked_disabled.png \
		../qdarkstyle/rc/branch_open.png \
		../qdarkstyle/rc/left_arrow.png \
		../qdarkstyle/rc/stylesheet-branch-end.png \
		../qdarkstyle/rc/undock.png \
		../qdarkstyle/rc/right_arrow.png \
		../qdarkstyle/rc/checkbox_indeterminate.png \
		../qdarkstyle/rc/checkbox_checked_focus.png \
		../qdarkstyle/rc/stylesheet-vline.png \
		../qdarkstyle/rc/radio_checked_focus.png \
		../qdarkstyle/rc/branch_closed.png \
		../qdarkstyle/rc/right_arrow_disabled.png \
		../qdarkstyle/rc/down_arrow_disabled.png \
		../qdarkstyle/rc/branch_open-on.png \
		../qdarkstyle/rc/radio_unchecked_focus.png \
		../qdarkstyle/rc/Vmovetoolbar.png \
		../qdarkstyle/rc/transparent.png \
		../qdarkstyle/rc/close.png \
		../qdarkstyle/rc/radio_checked_disabled.png \
		../qdarkstyle/rc/checkbox_checked.png \
		../qdarkstyle/rc/up_arrow_disabled.png \
		../qdarkstyle/rc/checkbox_indeterminate_focus.png \
		../qdarkstyle/rc/checkbox_unchecked_focus.png \
		../qdarkstyle/style.qss \
		${RC_FILE}

RESOURCE_CODE := resources.cc

SRC :=  main.cc  \
	mainwindow.cc \
	keyboard.cc \
	signals_handler.cc \
	bin_file_reader.cc \
	utils.cc \
	${MOC_FILES} \
	${RESOURCE_CODE}

OBJS := ${SRC:.cc=.o}



COVERAGE_HTML_DIR := ../COVERAGE_OUTPUT
LCOV=lcov
LCOV_FLAGS= --rc lcov_branch_coverage=1 \
	    --rc genhtml_branch_coverage=1 \
	    --rc geninfo_gcov_all_blocks=1 \
	    --directory .

GENHTML = genhtml
GENHTML_FLAGS = --branch-coverage

GPROF = gprof
PROFILING_OUTPUT = profiling_output.txt

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
  CXXFLAGS += -O3
else
  CXXFLAGS += -O0 -ggdb3
endif

ifeq ($(GCOV),1)
  CXXFLAGS += --coverage
endif

ifeq ($(GPROF),1)
  CXXFLAGS += -pg
endif

SCAN_BUILD = scan-build

all: ${TARGET}

${TARGET}: ${OBJS}
	-mkdir -p ${TARGET_DIR}
	${CXX} ${CXXFLAGS} -o ${TARGET} ${OBJS} ${LIBS}

${RESOURCE_CODE}: ${QT_STYLE_FILES}
	cd ../qdarkstyle && ${RCC} -o ../src/"$@" ${RC_FILE}


define specific_flags
 $(if $(findstring clang,$(CXX)),\
   $(if $(findstring "resources.o",$(1)),-Wno-exit-time-destructors -Wno-global-constructors,)\
   $(if $(findstring "moc_mainwindow.o",$(1)),-Wno-undefined-reinterpret-cast,)\
  )
endef

%.o: %.cc ${FORMS_HEADERS}
	${CXX} ${CXXFLAGS} $(call specific_flags,"$@") ${INCLUDES} -MD -c -o "$@" "$<"
	@cp $*.d $*.P; \
	   sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	       -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P; \
	   rm -f $*.d

moc_%.cc: %.hh ui_%.hh
	${MOC} ${QT_DEFINES} ${QT_INCLUDES:-isystem=-I} -o "$@" "$<" && \
	sed -i '1i // Avoid warnings on generated file\n#if !defined(__clang__)\n  #pragma GCC diagnostic error "-w"\n#endif\n' "$@"

ui_%.hh: %.ui
	${UIC} -o "$@" "$<" && \
	sed -i '1i // Avoid warnings on generated headers\n#if !defined(__clang__)\n  #pragma GCC system_header\n#endif\n' "$@"

scan-build:
	${SCAN_BUILD} -analyze-headers --use-c++=/usr/bin/clang++ --status-bugs --keep-going  make -B

${COVERAGE_HTML_DIR}:
	[ -d "$@" ] || mkdir -p "$@"

coverage: | ${COVERAGE_HTML_DIR}
	${MAKE} GCOV=1 -B CXX=g++
	${LCOV} ${LCOV_FLAGS} --zerocounters
	${MAKE} check
	${LCOV} ${LCOV_FLAGS} --no-external --capture --output-file "${TARGET}.info"
	${GENHTML} ${GENHTML_FLAGS} --output-directory="${COVERAGE_HTML_DIR}" "${TARGET}.info"

profiling:
	${MAKE} GPROF=1 -B CXX=g++
	${TARGET}
	${GPROF} "${TARGET}" gmon.out > "${PROFILING_OUTPUT}"

check: ${TARGET}
	${TARGET}


clean:
	rm -rf ${TARGET} ${OBJS} $(SRC:%.cc=$/%.P) ${MOC_FILES} ${FORMS_HEADERS} Makefile.vars ${RESOURCE_CODE} \
	       $(SRC:%.cc=%.gcda) $(SRC:%.cc=%.gcno) $(SRC:%.cc=%.info) $(SRC:%.cc=%.gcna) \
	       "${COVERAGE_HTML_DIR}"  "${TARGET}.info" gmon.out  "${PROFILING_OUTPUT}"


.PHONY: all clean scan-build coverage check profiling

.SUFFIXES:

-include $(SRC:%.cc=$/%.P)
