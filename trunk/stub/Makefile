
PREFIX = powerpc-eabi-

AR = $(PREFIX)ar
AS = $(PREFIX)as
CC = $(PREFIX)gcc
CXX = $(PREFIX)g++
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy
RANLIB = $(PREFIX)ranlib
STRIP = $(PREFIX)strip

MACHDEP = -mcpu=750 -mno-eabi -mhard-float
LDFLAGS = $(MACHDEP) -n -nostartfiles -nostdlib -Wl,-T,openstub.ld -L.
ASFLAGS = -D_LANGUAGE_ASSEMBLY -DHW_RVL

TARGET_LINKED = stub.elf
TARGET = stub.bin
OBJS = crt0.o

DEPDIR = .deps

all: $(TARGET)

%.o: %.s
	@echo " ASSEMBLE    $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

%.o: %.S
	@echo " ASSEMBLE    $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

$(TARGET_LINKED): $(OBJS)
	@echo " LINK        $@"
	@$(CC) -g -o $@ $(LDFLAGS) $(OBJS)

$(TARGET): $(TARGET_LINKED)
	@echo " OBJCOPY     $@"
	@$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(TARGET_LINKED) $(TARGET) $(OBJS) $(DEPDIR)
