.PHONY: clean runseq

PIC_NAME = homework.png
RUN_ARG = 1 -2 2 -2 2 800 400 $(PIC_NAME)

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

clean:
	rm -f seq simd
