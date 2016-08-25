NAME := tildifier
PARAMS := 

OPTS := 
LIBS := 
STD := c11

RELEASEOPTS := -O3
PROFILEOPTS := -pg -O3
DEBUGOPTS   := -g

COMPILER := cc
WERROR := -Werror
DEFAULTOPTS := -Wall -Wextra -pedantic $(WERROR) -D_POSIX_C_SOURCE=200809L -fno-rtti -fno-exceptions
DEFAULTLIBS := 

SRC := $(shell find . -path ./obj -prune -o -iname .svn -prune -o -iname \*.c -print)

clean : 
	rm -rf obj

obj/%.o : %.c makefile
	@mkdir -p obj
	@find . -path ./obj -path ./obj -prune -o -iname .svn -prune -o -type d ! -name . -exec mkdir -p ./obj/{} \;
	$(COMPILER) $(OPTS) $(RELEASEOPTS) $(DEFAULTOPTS) -std=$(STD) -MMD -MP -o $@ -c $<

obj/$(NAME) : $(SRC:%.c=obj/%.o)
	@mkdir -p obj
	$(COMPILER) $(OPTS) $(RELEASEOPTS) $(DEFAULTOPTS) -std=$(STD) -o $@ $^ $(LIBS) $(DEFAULTLIBS)
	
obj/%.op : %.c makefile
	@mkdir -p obj
	@find . -path ./obj -path ./obj -prune -o -iname .svn -prune -o -type d ! -name . -exec mkdir -p ./obj/{} \;
	$(COMPILER) $(OPTS) $(PROFILEOPTS) $(DEFAULTOPTS) -std=$(STD) -MMD -MP -o $@ -c $<

obj/$(NAME)_p : $(SRC:%.c=obj/%.op)
	@mkdir -p obj
	$(COMPILER) $(OPTS) $(PROFILEOPTS) $(DEFAULTOPTS) -std=$(STD) -o $@ $^ $(LIBS) $(DEFAULTLIBS)

obj/%.od : %.c makefile
	@mkdir -p obj
	@find . -path ./obj -path ./obj -prune -o -iname .svn -prune -o -type d ! -name . -exec mkdir -p ./obj/{} \;
	$(COMPILER) $(OPTS) $(DEBUGOPTS) $(DEFAULTOPTS) -std=$(STD) -MMD -MP -o $@ -c $<

obj/$(NAME)_d : $(SRC:%.c=obj/%.od)
	@mkdir -p obj
	$(COMPILER) $(OPTS) $(DEBUGOPTS) $(DEFAULTOPTS) -std=$(STD) -o $@ $^ $(LIBS) $(DEFAULTLIBS)
	
run : obj/$(NAME)
	@echo ./obj/$(NAME) $(PARAMS)
	@echo
	@./obj/$(NAME) $(PARAMS)
	
runp : obj/$(NAME)_p
	@echo ./obj/$(NAME)_p $(PARAMS)
	@echo
	@./obj/$(NAME)_p $(PARAMS)
	
rund : obj/$(NAME)_d
	@echo ./obj/$(NAME)_d $(PARAMS)
	@echo
	@./obj/$(NAME)_d $(PARAMS)

install : obj/$(NAME)
	sudo cp "obj/$(NAME)" "/usr/local/bin/$(NAME)"
	sudo chmod =rx "/usr/local/bin/$(NAME)"

release : obj/$(NAME)
debug : obj/$(NAME)_d
profile : obj/$(NAME)_p
all : release debug profile

-include $(SRC:%.c=obj/%.d)

.PHONY : run rund runp all clean release debug profile install
.DEFAULT_GOAL := run
