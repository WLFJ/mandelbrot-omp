.PHONY: clean runseq

PIC_NAME = homework.png
RUN_ARG = 1 -2 2 -2 2 800 400 $(PIC_NAME)
RUN_ARG_BIG = 1 -2 2 -2 2 8000 4000 $(PIC_NAME)

seq: sequential.c
	gcc -g -Wall -o $@ $< -lpng	 -O3

simd: simd.c
	gcc -g -Wall -o $@ $< -lpng -fopenmp -O3

runseq: seq
	./$< $(RUN_ARG)
	@xdg-open $(PIC_NAME)

runsimd: simd
	./$< $(RUN_ARG)
	@xdg-open $(PIC_NAME)

debsimd: simd.c
	gcc -g -Wall -o $@ $< -lpng -fopenmp -O3 -fverbose-asm

perfsimd: simd
	perf stat  ./$< $(RUN_ARG)

perfseq: seq
	perf stat  ./$< $(RUN_ARG)

clean:
	rm -f seq simd $(PIC_NAME) debsimd
