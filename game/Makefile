LIB_COMMON_DIR = ../libs
NEWLIBC_DIR = $(LIB_COMMON_DIR)/newlib

#GAME = typing
#GAME = nemu-pal
GAME = credits

SUB_GAME = hello

CC = gcc
CFLAGS = -m32 -MMD -Wall -Werror \
		 -fno-builtin -fno-omit-frame-pointer -fno-stack-protector \
		 -I./include -I../include -I$(NEWLIBC_DIR)/include
LDFLAGS = -melf_i386 #-Ttext=0x800000

CFLAGS += -I./src/$(GAME)/include

COMMON_FILES = $(shell find src/common -name "*.c")
GAME_FILES = $(shell find src/$(GAME) -name "*.c")
SUB_FILES = $(shell find src/$(SUB_GAME) -name "*.c")

CFILES = $(COMMON_FILES) $(GAME_FILES)
OBJS = $(CFILES:.c=.o) 

SUB_CFILES = $(COMMON_FILES) $(SUB_FILES)
SUB_OBJS = $(SUB_CFILES:.c=.o) 

NEWLIBC = $(NEWLIBC_DIR)/libc.a

game: main_game sub_game
	cp main_game game
	dd if=/dev/null of=game seek=2048
	cat `find ./data/$(GAME)/ -type f | sort` >> game
	dd if=sub_game of=game seek=65536

main_game: $(OBJS)
	ld $(LDFLAGS) -e game_init -o main_game $(OBJS) $(NEWLIBC)
	../scripts/objdump4nemu-i386 main_game -d > main_game_elf.txt

sub_game: $(SUB_OBJS)
	ld $(LDFLAGS) -e game_init -o sub_game $(SUB_OBJS) $(NEWLIBC)
	../scripts/objdump4nemu-i386 sub_game -d > sub_game_elf.txt

-include $(OBJS:.o=.d)
-include $(SUB_OBJS:.o=.d)

.PHONY: clean

clean:
	-rm -f game $(OBJS) $(OBJS:.o=.d) $(SUB_OBJS) $(SUB_OBJS:.o=.d) \
	main_game main_game_elf.txt sub_game sub_game_elf.txt
