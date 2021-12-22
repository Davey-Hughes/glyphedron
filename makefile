BUILD := ./build
OBJDIR := $(BUILD)/objects
DEPDIR := $(OBJDIR)/.deps

BASE_FLAGS := -Wall -Werror -Wextra -pedantic-errors

# linker
LD := g++-11
# linker flags
LDFLAGS := -lm -lncurses
# linker flags: libraries to link (e.g. -lfoo)
LDLIBS :=
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

CTARGET := c_term_shapes
CC := gcc-11
CFLAGS := -std=c11 $(BASE_FLAGS)
CINCLUDE := -Ic_include
CSRC := $(wildcard c_src/*.c)
COBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(CSRC)))
CDEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(CSRC)))

CXXTARGET := term_shapes
CXX := g++-11
CXXFLAGS := -std=c++17 $(BASE_FLAGS)
SYSINCLUDE := -isystem /usr/local/include/eigen3
CXXINCLUDE := -Iinclude/
CXXSRC := $(wildcard src/*.cc)
CXXOBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(CXXSRC)))
CXXDEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(CXXSRC)))

# compile C source files
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CINCLUDE) -c -o $@
# compile C++ source files
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CXXINCLUDE) $(SYSINCLUDE) -c -o $@
# link object files to binary
LINK.o = $(LD) $(LDFLAGS) -o $@
# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

.PHONY: all c_impl clean # debug release

all: $(CXXTARGET)

c_impl: $(CTARGET)

# debug: CXXFLAGS += -DDEBUG -g
# debug: all

# release: CXXFLAGS += -O3
# release: all

clean:
	rm -rvf $(BUILD)
	rm -rvf $(CTARGET)
	rm -rvf $(CXXTARGET)
	rm -f log.txt

$(CTARGET): $(COBJS)
	$(LINK.o) $^

$(CXXTARGET): $(CXXOBJS)
	$(LINK.o) $^

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(shell mkdir -p $(dir $(COBJS)) >/dev/null)
	$(shell mkdir -p $(dir $(CDEPS)) >/dev/null)
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cc
$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	$(shell mkdir -p $(dir $(CXXOBJS)) >/dev/null)
	$(shell mkdir -p $(dir $(CXXDEPS)) >/dev/null)
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(CDEPS)
-include $(CCDEPS)
