CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

BUILD = build
CHAT  = ../chat
LINKER_SCRIPT = STM32F303K8Tx_FLASH.ld

MCUFLAGS = -mcpu=cortex-m4 -mthumb
CPPFLAGS = -DSTM32F303x8 \
           -Idrivers \
           -I$(CHAT) \
           -ICMSIS/Core/Include \
           -ICMSIS/Device/Include

CFLAGS = $(MCUFLAGS) -std=c11 -Wall -Wextra -O0 -g3 \
         -ffunction-sections -fdata-sections -MMD -MP

LDFLAGS = $(MCUFLAGS) -T$(LINKER_SCRIPT) \
          --specs=nano.specs --specs=nosys.specs \
          -Wl,--gc-sections -Wl,-Map=$(BUILD)/firmware.map

C_SOURCES = src/main.c \
            system/system_stm32f3xx.c \
            drivers/time.c \
            drivers/clock.c \
            drivers/uart.c \
            src/ring_buffer.c

CHAT_SOURCES = $(CHAT)/chat.c \
               $(CHAT)/framing.c

ASM_SOURCES = startup/startup_stm32f303x8.s

OBJS = $(addprefix $(BUILD)/,$(C_SOURCES:.c=.o) $(ASM_SOURCES:.s=.o)) \
       $(patsubst $(CHAT)/%.c,$(BUILD)/chat/%.o,$(CHAT_SOURCES))

all: $(BUILD)/firmware.bin

$(BUILD)/%.o: %.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/chat/%.o: $(CHAT)/%.c Makefile
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s Makefile
	mkdir -p $(dir $@)
	$(CC) $(MCUFLAGS) -g3 -c $< -o $@

$(BUILD)/firmware.elf: $(OBJS) $(LINKER_SCRIPT) Makefile
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD)/firmware.bin: $(BUILD)/firmware.elf
	$(OBJCOPY) -O binary $< $@

flash: $(BUILD)/firmware.elf
	openocd -f interface/stlink.cfg -f target/stm32f3x.cfg \
	        -c "program $< verify reset exit"

clean:
	rm -rf -- $(BUILD)

-include $(OBJS:.o=.d)

.PHONY: all flash clean
