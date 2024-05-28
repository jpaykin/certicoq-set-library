PATH_TO_CERTICOQ=../certicoq/plugin/runtime

all: SetBenchmarks.o RBT.o glue.o CoqSet.o $(PATH_TO_CERTICOQ)/gc_stack.o $(PATH_TO_CERTICOQ)/prim_int63.o
#	make -C $(PATH_TO_CERTICOQ)
	clang++ -o SetBenchmarks $^ -lm

$(PATH_TO_CERTICOQ)/%.o: $(PATH_TO_CERTICOQ)/%.c
	make -C $(PATH_TO_CERTICOQ)

clean:
	rm *.o *.vo* *.glob CoqSet RBT.* glue.*

RBT.c: MSetImplementation.v
	coqc $<

glue.c: MSetImplementation.v
	coqc $<

%.o: %.c
	clang -c -o $@ -O2 -Wno-everything -fomit-frame-pointer -I $(PATH_TO_CERTICOQ) $<

%.o: %.cpp
	clang++ -c -o $@ -O2 -fomit-frame-pointer -I $(PATH_TO_CERTICOQ) $<
