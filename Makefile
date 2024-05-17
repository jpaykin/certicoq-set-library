PATH_TO_CERTICOQ=../certicoq

all:
	coqc MSetImplementation.v
	make -C $(PATH_TO_CERTICOQ)/plugin/runtime all
	clang -o RBT.o -Wno-everything -O2 -fomit-frame-pointer -I $(PATH_TO_CERTICOQ)/plugin/runtime -c RBT.c
	clang -o glue.o -Wno-everything -O2 -fomit-frame-pointer -I  $(PATH_TO_CERTICOQ)/plugin/runtime -c glue.c
	clang++ -o CoqSet.o -O2 -fomit-frame-pointer -I  $(PATH_TO_CERTICOQ)/plugin/runtime -c CoqSet.cpp
	clang++ -o CoqSet  $(PATH_TO_CERTICOQ)/plugin/runtime/gc_stack.o  $(PATH_TO_CERTICOQ)/plugin/runtime/prim_int63.o \
				RBT.o glue.o CoqSet.o -lm


clean:
	rm *.o *.vo* *.glob CoqSet RBT.* glue.*