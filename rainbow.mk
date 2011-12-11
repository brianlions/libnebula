#
# rainbow.mk
#
#  Created on: Aug 21, 2011
#  Author:     Brian Y. ZHANG
#  Email:      brianlions at gmail dot com
#

ifndef NO_COLOR
DEP_COLOR_A="\033[36m"
DEP_COLOR_B="\033[36m"
OBJ_COLOR_A="\033[32m"
OBJ_COLOR_B="\033[32m"
EXE_COLOR_A="\033[1;32m"
EXE_COLOR_B="\033[1;32m"
AR_COLOR_A ="\033[35m"
AR_COLOR_B ="\033[35m"
END_COLOR  ="\033[0m"
COLOR_DEP  =@printf '%b\t%b\n' $(DEP_COLOR_A)CPP$(END_COLOR) $(DEP_COLOR_B)$@$(END_COLOR) &&
COLOR_OBJ  =@printf '%b\t%b\n' $(OBJ_COLOR_A)CC$(END_COLOR) $(OBJ_COLOR_B)$@$(END_COLOR) &&
COLOR_EXE  =@printf '%b\t%b\n' $(EXE_COLOR_A)LINK$(END_COLOR) $(EXE_COLOR_B)$@$(END_COLOR) &&
COLOR_AR   =@printf '%b\t%b\n' $(AR_COLOR_A)AR$(END_COLOR) $(AR_COLOR_B)$@$(END_COLOR) &&
COLORFUL='yes'
else
COLOR_DEP=
COLOR_OBJ=
COLOR_EXE=
COLOR_AR=
COLORFUL=
endif
