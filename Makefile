NASM :=	nasm -g -f elf64 -F dwarf $^

CFLAGS += -m64 -std=c99 -Wall -Wshadow -Wpointer-arith -Wcast-qual \
          -Wstrict-prototypes -fPIC -g -O2 -masm=intel -march=ivybridge

H_SRCS := fe_convert.h \
          fe10.h \
          fe12.h \
          ge.h \
          mxcsr.h \
          fe51.h
ASM_SCRS := fe12_mul.asm \
            fe12_squeeze.asm \
            ge_double.asm \
            ge_add.asm \
            select.asm \
            ladder.asm
C_SRCS := mxcsr.c \
          fe10.c \
          fe12_old.c \
          fe_convert.c \
          ge.c \
          scalarmult.c \
          fe51_invert.c
S_SRCS := fe51_mul.S \
          fe51_nsquare.S \
          fe51_pack.S

ASM_OBJS := $(ASM_SCRS:%.asm=%.o)
C_OBJS := $(C_SRCS:%.c=%.o)
S_OBJS := $(S_SRCS:%.S=%.o)


all: libref12.so

%.o: %.asm
	$(NASM) -l $(patsubst %.o,%.lst,$@) -o $@ $<

libref12.so: $(ASM_OBJS) $(C_OBJS) $(S_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: check
check: libref12.so
	sage -python test_all.py -v $(TESTNAME)

.PHONY: clean
clean:
	$(RM) *.o *.gch *.a *.out *.so *.d *.lst

%.d: %.asm
	$(NASM) -MT $(patsubst %.d,%.o,$@) -M $< >$@

%.d: %.c
	$(CC) $(CFLAGS) -M $< >$@

include $(ASM_SCRS:%.asm=%.d)
include $(C_SRCS:%.c=%.d)


# ===== Rules for benchmarking setup below this line =====

bench.out: bench.o $(ASM_OBJS) $(C_OBJS) $(S_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: bench
bench: bench.out
	@echo "[!] Dont forget to"
	@echo "    - Disable TurboBoost;"
	@echo "    - Disable HyperThreading cores; and"
	@echo "    - Set the CPU to 'performance'."
	./bench.out | sort | head -n 500 | tail -n 1