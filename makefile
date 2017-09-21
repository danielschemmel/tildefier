NAME := tildifier
PARAMS := 

OPTIMIZE := -O3 -flto

COMPILER := $(shell which musl-gcc >/dev/null 2>/dev/null && echo musl-gcc -static || echo cc)
WERROR := -Werror
OPTS := -std=c11 -Wall -Wextra -pedantic $(WERROR) -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 $(OPTIMIZE)
LIBS := 

SRC := $(shell find . -path ./obj -prune -o -name "'.?*'" -prune -o -iname \*.c -print)
VERSION := $(shell echo "$$(git rev-list --count HEAD).$$(git rev-parse --short --verify HEAD)")
DIRTY := $(shell if [[ -n "$$(git status --porcelain)" ]] ; then echo '.dirty' ; fi)
OPTS += -DVERSION="$(VERSION)$(DIRTY)"

clean : 
	rm -rf obj

obj/%.o : %.c makefile
	@mkdir -p obj
	@find . -path ./obj -path ./obj -prune -o -name "'.?*'" -prune -o -type d ! -name . -exec mkdir -p ./obj/{} \;
	$(COMPILER) $(OPTS) -MMD -MP -o $@ -c $<

obj/$(NAME) : $(SRC:%.c=obj/%.o)
	@mkdir -p obj
	$(COMPILER) $(OPTS) -o $@ $^ $(LIBS)

all : obj/$(NAME)

-include $(SRC:%.c=obj/%.d)

.PHONY : all clean
.DEFAULT_GOAL := all
