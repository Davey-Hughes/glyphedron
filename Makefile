TARGET := term_shapes
BUILD := ./build
OBJDIR := $(BUILD)/objects
BINDIR := $(BUILD)/bin
DEPDIR := $(OBJDIR)/.deps
BASE_SRC := src

BASE_FLAGS := -Wall -Werror -Wextra -pedantic-errors

# linker
LD := clang
# set by the test-asan target, empty otherwise
SANFLAGS :=
# linker flags
LDFLAGS := $(SANFLAGS)
# linker flags: libraries to link (e.g. -lfoo)
LDLIBS := -lm -lncurses
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

CTARGET := c_term_shapes
CC := clang
# ssize_t, nanosleep, clock_gettime and CLOCK_PROCESS_CPUTIME_ID are POSIX, not
# ISO C. -std=c11 (as opposed to -std=gnu11) defines __STRICT_ANSI__, which
# hides them in glibc unless a feature test macro asks for them explicitly.
CFLAGS := -std=c11 -D_POSIX_C_SOURCE=200809L $(BASE_FLAGS) $(SANFLAGS)
CINCLUDE := -I$(BASE_SRC)/c/include
CSRC := $(wildcard $(BASE_SRC)/c/src/*.c)
COBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(CSRC)))
CDEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(CSRC)))

TESTTARGET := c_term_shapes_tests
# the tests never touch ncurses, so they link libm only
TESTLDLIBS := -lm

# modules that need neither ncurses nor main(), so tests can link them.
# raster.c joins this list in Task 6
CORESRC := $(addprefix $(BASE_SRC)/c/src/, \
	vector.c occlusion.c convex_occlusion.c occlude_approx.c init.c)
COREOBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(CORESRC)))

TESTSRC := $(wildcard $(BASE_SRC)/c/tests/*.c)
TESTOBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(TESTSRC)))
TESTDEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(TESTSRC)))

# sentinel files recording the flags each object and the binary were last built
# with, so that switching between e.g. debug_c and release_c invalidates them.
# without this, flags live only in the recipe and make sees nothing stale
CFLAGSFILE := $(OBJDIR)/.cflags
LDFLAGSFILE := $(OBJDIR)/.ldflags

# compile C source files
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CINCLUDE) -c -o $@
# link object files to binary; libraries must follow the objects that need them
LINK.o = $(LD) $(LDFLAGS) -o $@
# precompile step
PRECOMPILE =
# postcompile step; the touch keeps the object newer than the dep file it just
# generated, otherwise every object looks stale against its own .d on the next
# run and make rebuilds the whole tree
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

.PHONY: all c debug_c release_c test test-asan clean FORCE

all: c

c: $(BINDIR)/$(CTARGET)
	ln -sf $(BINDIR)/$(CTARGET) $(TARGET)

debug_c: CFLAGS += -DDEBUG -g
debug_c: c

release_c: CFLAGS += -O3
release_c: c

test: $(BINDIR)/$(TESTTARGET)
	$(BINDIR)/$(TESTTARGET)

test-asan:
	@$(MAKE) --no-print-directory BUILD=./build/asan \
		SANFLAGS='-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -g' \
		test

clean:
	rm -rvf $(BUILD)
	rm -vf $(TARGET)
	rm -f log.txt

#
# the sentinels depend on FORCE so their recipes run on every build, but each
# recipe rewrites its file only when the flags differ from the recorded ones.
# an unchanged sentinel keeps its old mtime, so objects are not invalidated.
#
# these must not be created at parse time: `make clean` would then leave the
# build directory behind, and `make clean all` would delete a sentinel make had
# already stat'd. a normal rule is remade at the right point in the walk
#
# target-specific CFLAGS (debug_c, release_c) reach here because GNU make
# passes them down to prerequisites, and the sentinel is a prerequisite of
# every object
#
FORCE:

$(CFLAGSFILE): FORCE
	@mkdir -p $(@D)
	@printf '%s' '$(CC) $(CFLAGS) $(CINCLUDE)' | cmp -s - $@ || \
		printf '%s' '$(CC) $(CFLAGS) $(CINCLUDE)' > $@

$(LDFLAGSFILE): FORCE
	@mkdir -p $(@D)
	@printf '%s' '$(LD) $(LDFLAGS) $(LDLIBS) $(TESTLDLIBS)' | cmp -s - $@ || \
		printf '%s' '$(LD) $(LDFLAGS) $(LDLIBS) $(TESTLDLIBS)' > $@

# $(COBJS) rather than $^, so the sentinel is not handed to the linker
$(BINDIR)/$(CTARGET): $(COBJS) $(LDFLAGSFILE)
	-@mkdir -p $(BINDIR)
	$(LINK.o) $(COBJS) $(LDLIBS)

$(BINDIR)/$(TESTTARGET): $(COREOBJS) $(TESTOBJS) $(LDFLAGSFILE)
	-@mkdir -p $(BINDIR)
	$(LINK.o) $(COREOBJS) $(TESTOBJS) $(TESTLDLIBS)

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d $(CFLAGSFILE)
	$(shell mkdir -p $(dir $(COBJS)) >/dev/null)
	$(shell mkdir -p $(dir $(CDEPS)) >/dev/null)
	$(shell mkdir -p $(dir $(TESTOBJS)) >/dev/null)
	$(shell mkdir -p $(dir $(TESTDEPS)) >/dev/null)
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(CDEPS)
-include $(TESTDEPS)
