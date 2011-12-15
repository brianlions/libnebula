#
# Makefile
#
#  Created on: Nov 24, 2011
#  Author:     Brian Y. ZHANG
#  Email:      brianlions at gmail dot com
#

.PHONY: all clean distclean echo runtest valgrindtest

-include rainbow.mk

CC                 := gcc
CXX                := g++
CFLAGS             := -O2 -Wall
CXXFLAGS           := -O2 -Wall
CFLAGS_DBG         := -O0 -g -Wall
CXXFLAGS_DBG       := -O0 -g -Wall
AR                 := ar

root_dir           := .
inc_dir            := $(root_dir)/include
src_dir            := $(root_dir)/src
bin_dir            := $(root_dir)/bin
test_dir           := $(root_dir)/test
poc_dir            := $(root_dir)/poc

include_paths      := $(inc_dir) $(src_dir) ~/local/include ~/lab/include /usr/local/include /opt/include
library_paths      := $(bin_dir) ~/local/lib ~/lab/lib /usr/local/lib /opt/lib
libraries          := rt pthread gtest

static_lib         := libnebula.a
static_lib_dbg     := libnebula_debug.a

ifndef NO_COLOR
MY_DEP  =@printf '%b\t%b\n' $(DEP_COLOR_A)CPP$(END_COLOR) $(DEP_COLOR_B)$(1)$(END_COLOR) &&
MY_OBJ  =@printf '%b\t%b\n' $(OBJ_COLOR_A)CC$(END_COLOR) $(OBJ_COLOR_B)$(1)$(END_COLOR) &&
MY_EXE  =@printf '%b\t%b\n' $(EXE_COLOR_A)LINK$(END_COLOR) $(EXE_COLOR_B)$(1)$(END_COLOR) &&
MY_AR   =@printf '%b\t%b\n' $(AR_COLOR_A)AR$(END_COLOR) $(AR_COLOR_B)$(1)$(END_COLOR) &&
else
MY_DEP  =
MY_OBJ  =
MY_EXE  =
MY_AR   =
endif

ifdef PRETTY_MESSAGE
CFLAGS       += -D USE_PRETTY_MESSAGE
CXXFLAGS     += -D USE_PRETTY_MESSAGE
CFLAGS_DBG   += -D USE_PRETTY_MESSAGE
CXXFLAGS_DBG += -D USE_PRETTY_MESSAGE
endif

comp.cxx      = $(MY_OBJ) $(CXX) $(CXXFLAGS) $(addprefix -I ,$(include_paths)) -c -o $(1) $(2)
comp_dbg.cxx  = $(MY_OBJ) $(CXX) $(CXXFLAGS_DBG) $(addprefix -I ,$(include_paths)) -c -o $(1) $(2)
dep.cxx       = $(MY_DEP) $(CXX) $(CXXFLAGS) $(addprefix -I ,$(include_paths)) -MM -MF $(1) -MT $(basename $(1)).o $(2)
dep_dbg.cxx   = $(MY_DEP) $(CXX) $(CXXFLAGS_DBG) $(addprefix -I ,$(include_paths)) -MM -MF $(1) -MT $(basename $(1)).o $(2)
link.cxx      = $(MY_EXE) $(CXX) $(CXXFLAGS) -o $(1) $(2) $(addprefix -L ,$(library_paths)) $(addprefix -l ,$(libraries))
link_dbg.cxx  = $(MY_EXE) $(CXX) $(CXXFLAGS_DBG) -o $(1) $(2) $(addprefix -L ,$(library_paths)) $(addprefix -l ,$(libraries))
lib_static.ar = $(MY_AR) $(AR) -r $(1) $(2)

#define comp_cxx_template
#$(1): $(2)
#	$$(CXX) $$(CXXFLAGS) $$(addprefix -I ,$$(include_paths)) -c -o $$@ $$<
#endef

#----------------------------------------------------------------------------------------------------------------------

core_main_srcs     := $(wildcard $(src_dir)/*_main.cc)
core_main_deps     := $(patsubst $(src_dir)/%.cc,$(bin_dir)/%.d,$(core_main_srcs))
core_main_objs     := $(patsubst $(src_dir)/%.cc,$(bin_dir)/%.o,$(core_main_srcs))
core_main_deps_dbg := $(patsubst $(src_dir)/%.cc,$(bin_dir)/%_dbg.d,$(core_main_srcs))
core_main_objs_dbg := $(patsubst $(src_dir)/%.cc,$(bin_dir)/%_dbg.o,$(core_main_srcs))

core_srcs          := $(filter-out $(core_main_srcs),$(wildcard $(src_dir)/*.cc))
core_deps          := $(patsubst $(src_dir)/%.cc,$(bin_dir)/%.d,$(core_srcs))
core_objs          := $(patsubst $(src_dir)/%.cc,$(bin_dir)/%.o,$(core_srcs))
core_deps_dbg      := $(patsubst $(src_dir)/%.cc,$(bin_dir)/%_dbg.d,$(core_srcs))
core_objs_dbg      := $(patsubst $(src_dir)/%.cc,$(bin_dir)/%_dbg.o,$(core_srcs))

core_exes          := $(patsubst $(src_dir)/%_main.cc,$(bin_dir)/%.exe,$(core_main_srcs))
core_exes_dbg      := $(patsubst $(src_dir)/%_main.cc,$(bin_dir)/%-dbg.exe,$(core_main_srcs))

poc_srcs           := $(wildcard $(poc_dir)/*.cc)
poc_deps           := $(patsubst $(poc_dir)/%.cc,$(bin_dir)/%.d,$(poc_srcs))
poc_objs           := $(patsubst $(poc_dir)/%.cc,$(bin_dir)/%.o,$(poc_srcs))
poc_deps_dbg       := $(patsubst $(poc_dir)/%.cc,$(bin_dir)/%_dbg.d,$(poc_srcs))
poc_objs_dbg       := $(patsubst $(poc_dir)/%.cc,$(bin_dir)/%_dbg.o,$(poc_srcs))

poc_exes           := $(patsubst $(poc_dir)/%.cc,$(bin_dir)/%.exe,$(poc_srcs))
poc_exes_dbg       := $(patsubst $(poc_dir)/%.cc,$(bin_dir)/%-dbg.exe,$(poc_srcs))

test_main_srcs     := $(wildcard $(test_dir)/*_main.cc)
test_main_deps     := $(patsubst $(test_dir)/%.cc,$(bin_dir)/%.d,$(test_main_srcs))
test_main_objs     := $(patsubst $(test_dir)/%.cc,$(bin_dir)/%.o,$(test_main_srcs))

test_srcs          := $(filter-out $(test_main_srcs),$(wildcard $(test_dir)/*.cc))
test_deps          := $(patsubst $(test_dir)/%.cc,$(bin_dir)/%.d,$(test_srcs))
test_objs          := $(patsubst $(test_dir)/%.cc,$(bin_dir)/%.o,$(test_srcs))

test_exes          := $(patsubst $(test_dir)/%.cc,$(bin_dir)/%.exe,$(test_srcs))

#----------------------------------------------------------------------------------------------------------------------

all: $(core_exes) $(core_exes_dbg) $(poc_exes) $(poc_exes_dbg) $(test_exes) \
     $(static_lib) $(static_lib_dbg)

$(core_main_deps) $(core_main_objs) $(core_main_deps_dbg) $(core_main_objs_dbg) \
$(core_deps) $(core_objs) $(core_deps_dbg) $(core_objs_dbg) \
$(core_exes) $(core_exes_dbg) \
$(poc_deps) $(poc_objs) $(poc_deps_dbg) $(poc_objs_dbg) \
$(poc_exes) $(poc_exes_dbg) \
$(test_main_deps) $(test_main_objs) \
$(test_deps) $(test_objs) \
$(test_exes): | $(bin_dir)

-include $(core_main_deps) $(core_main_deps_dbg) \
         $(core_deps) $(core_deps_dbg) \
         $(poc_deps) $(poc_deps_dbg) \
         $(test_main_deps) $(test_deps)

runtest: $(test_exes)
	./unitTestRun.sh $^
valgrindtest: $(test_exes)
	./unitTestRun.sh --valgrind $^

# generated from source files
$(bin_dir)/%.d: $(src_dir)/%.cc
	$(call dep.cxx,$@,$<)
$(bin_dir)/%.o: $(src_dir)/%.cc
	$(call comp.cxx,$@,$<)
$(bin_dir)/%_dbg.d: $(src_dir)/%.cc
	$(call dep_dbg.cxx,$@,$<)
$(bin_dir)/%_dbg.o: $(src_dir)/%.cc
	$(call comp_dbg.cxx,$@,$<)
# generated from test source files
$(bin_dir)/%.d: $(test_dir)/%.cc
	$(call dep_dbg.cxx,$@,$<)
$(bin_dir)/%.o: $(test_dir)/%.cc
	$(call comp_dbg.cxx,$@,$<)
# generated from poc sourc files
$(bin_dir)/%.d: $(poc_dir)/%.cc
	$(call dep_dbg.cxx,$@,$<)
$(bin_dir)/%.o: $(poc_dir)/%.cc
	$(call comp_dbg.cxx,$@,$<)
$(bin_dir)/%_dbg.d: $(poc_dir)/%.cc
	$(call dep_dbg.cxx,$@,$<)
$(bin_dir)/%_dbg.o: $(poc_dir)/%.cc
	$(call comp_dbg.cxx,$@,$<)
# executables
$(bin_dir)/%.exe: $(bin_dir)/%_main.o $(core_objs)
	$(call link.cxx,$@,$^)
$(bin_dir)/%-dbg.exe: $(bin_dir)/%_main_dbg.o $(core_objs_dbg)
	$(call link_dbg.cxx,$@,$^)
$(bin_dir)/%_t.exe: $(bin_dir)/%_t.o $(test_main_objs) $(core_objs_dbg)
	$(call link_dbg.cxx,$@,$^)
$(bin_dir)/%.exe: $(bin_dir)/%.o $(core_objs)
	$(call link.cxx,$@,$^)
$(bin_dir)/%-dbg.exe: $(bin_dir)/%_dbg.o $(core_objs_dbg)
	$(call link_dbg.cxx,$@,$^)
# static lib files
$(static_lib): $(core_objs)
	$(call lib_static.ar,$@,$^)
$(static_lib_dbg): $(core_objs_dbg)
	$(call lib_static.ar,$@,$^)

#----------------------------------------------------------------------------------------------------------------------

$(bin_dir):
	@-mkdir -p $@
clean:
	-rm -f $(bin_dir)/*.o $(bin_dir)/*.exe $(static_lib) $(static_lib_dbg)
distclean:
	-rm -rf $(bin_dir) $(static_lib) $(static_lib_dbg)
echo:
	#
	# core_main_srcs:     $(core_main_srcs)
	# core_main_deps:     $(core_main_deps)
	# core_main_objs:     $(core_main_objs)
	# core_main_deps_dbg: $(core_main_deps_dbg)
	# core_main_objs_dbg: $(core_main_objs_dbg)
	#
	# core_srcs:          $(core_srcs)
	# core_deps:          $(core_deps)
	# core_objs:          $(core_objs)
	# core_deps_dbg:      $(core_deps_dbg)
	# core_objs_dbg:      $(core_objs_dbg)
	#
	# core_exes:          $(core_exes)
	# core_exes_dbg:      $(core_exes_dbg)
	#
	# test_main_srcs:     $(test_main_srcs)
	# test_main_deps:     $(test_main_deps)
	# test_main_objs:     $(test_main_objs)
	#
	# test_srcs:          $(test_srcs)
	# test_deps:          $(test_deps)
	# test_objs:          $(test_objs)
	#
	# test_exes:          $(test_exes)
	#

