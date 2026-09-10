CC = riscv32-unknown-elf-gcc
AS = riscv32-unknown-elf-as
OBJCOPY = riscv32-unknown-elf-objcopy
OBJDUMP = riscv32-unknown-elf-objdump

SOC_DIR = ../libs/soc
SOC_LIB = $(SOC_DIR)/build/libsoc.a
SOC_INC = $(SOC_DIR)/include

CPU_FLAGS = \
    -march=rv32i \
    -mabi=ilp32

CFLAGS = \
    $(CPU_FLAGS) \
    -O2 \
    -mcmodel=medany \
    -I$(SOC_INC) \
    -fno-common \
    -fvisibility=hidden \
    -nostdlib \
    -Wall \
    -Wextra \
    -Werror

CRT0 = crt0.S
LDSCRIPT = link.ld

OBJS = \
    build/crt0.o \
    build/main.o

.PHONY: all clean $(SOC_LIB)

all: build/app build/app.bin build/app.dis build/app.mem

build/app.dis: build/app
	$(OBJDUMP) -SD $< > $@

build/app.bin: build/app
	$(OBJCOPY) -O binary $< $@

build/app.mem: build/app.bin
	bin_mem_converter.py $< $@

build/app: $(OBJS) build/crt0.o $(SOC_LIB)
	$(CC) $(CFLAGS) $(OBJS) $(SOC_LIB) -T $(LDSCRIPT) -o $@

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/crt0.o: $(CRT0) | build
	$(AS) $(CPU_FLAGS) -c $< -o $@

$(SOC_LIB):
	make -C $(SOC_DIR)

build:
	mkdir -p build

clean:
	rm -rf build
	make -C $(SOC_DIR) clean
