# This makefile is written for gcc running under Solaris on a SPARCstation.
# To compile T-VPack on other systems, you may have to change:
# (1) CC to the name of your C compiler.
# (2) OPT_FLAGS should be changed to whatever options turn on maximum 
#     optimization in your compiler.

CC = gcc
#CC = purify gcc
#CC = g++


# Overly strict flags line below.  Lots of useless warnings, but can
# let you look for redudant declarations.
# To avoid redundant declarations here I use -D__STDC instead of
# -D__USE_FIXED_PROTOTYPES, but that means some prototypes are missing.

#FLAGS = -Wall -Wpointer-arith -Wcast-qual -Wstrict-prototypes -O -D__STDC__ -ansi -pedantic -Wredundant-decls -Wmissing-prototypes -Wshadow -Wcast-align -D_POSIX_SOURCE
 
#Flags to be passed to the compiler.  First is for strict warnings,
#second for interactive debugging and third for optimization.

#-D_POSIX_SOURCE stops extra declarations from being included in math.h
#and causing -Wshadow to complain about conflicts with y1 in math.h
#(Bessel function 1 of the second kind)
 
#WARN_FLAGS = -Wall -Wpointer-arith -Wcast-qual -Wstrict-prototypes -O -D__USE_FIXED_PROTOTYPES__ -ansi -pedantic -Wmissing-prototypes -Wshadow -Wcast-align -D_POSIX_SOURCE

DEBUG_FLAGS = -g
OPT_FLAGS = -O2

FLAGS = $(DEBUG_FLAGS)
#FLAGS = $(OPT_FLAGS)
#FLAGS = $(OPT_FLAGS) $(DEBUG_FLAGS)
#FLAGS = $(WARN_FLAGS)


#Useful flags on HP machines.
#DEBUG_FLAGS = -Aa -g
#OPT_FLAGS = -Aa +O3

EXE = main

OBJ = main.o util.o ff_pack.o read_blif.o  heapsort.o tree.o output_clustering.o asap.o check_precedence.o queue.o dummy_tree_place.o domain.o cost.o global_route.o check_route.o part_place.o model.o energy.o grow.o detail_route.o tree_energy.o list.o alap.o rename_blocks.o

SRC = main.c util.c ff_pack.c read_blif.c heapsort.c tree.c output_clustering.c asap.c check_precedence.c queue.c dummy_tree_place.c domain.c cost.c global_route.c check_route.c part_place.c model.c energy.c grow.c detail_route.c tree_energy.c list.c alap.c rename_blocks.c

H = util.h main.h globals.h read_blif.h ff_pack.h heapsort.h tree.h output_clustering.h asap.h check_precedence.h queue.h dummy_tree_place.h domain.h cost.h check_route.h global_route.h part_place.h model.h energy.h grow.h detail_route.h tree_energy.h list.h alap.h rename_blocks.h

LIB = -lm


$(EXE): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(EXE) $(LIB)

clean:
	rm -rf $(OBJ) $(EXE)

assign78.tar: $(SRC) $(H) makefile test
	tar -cvf assign78.tar $(SRC) $(H) makefile test/makefile test/*.blif

%.o:	%.c $(H)
	$(CC) -c $(FLAGS) $*.c



