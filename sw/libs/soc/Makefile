CC = riscv32-unknown-elf-gcc
AR = riscv32-unknown-elf-ar

CFLAGS ?= \
    -march=rv32i \
    -mabi=ilp32 \
    -mcmodel=medany \
    -fno-common \
    -fvisibility=hidden \
    -nostdlib \
    -Iinclude/soc \
    -Wall \
    -Wextra \
    -Werror

OBJS = \
    build/data_ram.o \
    build/gpio.o \
    build/reg.o \
    build/timer.o \
    build/uart.o \
    build/ethernet.o

.PHONY: all clean

all: build/libsoc.a

build/libsoc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build
