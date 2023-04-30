.PHONY: clean runseq

PIC_NAME = homework.png
RUN_ARG = 1 -2 2 -2 2 800 400 $(PIC_NAME)
RUN_ARG_BIG = 1 -2 2 -2 2 8000 4000 $(PIC_NAME)
OMP_FILE=sy2206220_omp.cpp

seq: sequential.c
	gcc -g -Wall -o $@ $< -lpng	 -O3

simd: $(OMP_FILE)
	g++ -g -Wall -o $@ $< -lpng -fopenmp -O3 -march=native

runseq: seq
	./$< $(RUN_ARG)
	@xdg-open $(PIC_NAME)

runsimd: simd
	./$< $(RUN_ARG)
	@xdg-open $(PIC_NAME)

debsimd: $(OMP_FILE)
	g++ -g -Wall -o $@ $< -lpng -fopenmp -O3 -fverbose-asm

perfsimd: simd
	perf stat  ./$< $(RUN_ARG)

prsimd: simd
	perf record -g ./$< $(RUN_ARG)
	perf report

perfsimdbig: simd
	perf stat  ./$< $(RUN_ARG_BIG)

perfseq: seq
	perf stat  ./$< $(RUN_ARG)

clean:
	rm -f seq simd $(PIC_NAME) debsimd perf.data perf.data.old
