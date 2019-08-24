SRC := src
INCLUDE := include

bin/slowjs: $(SRC)/*.c  $(INCLUDE)/slowjs/*.h
	@rm -rf bin && mkdir bin
	$(CC) -I $(INCLUDE) $(SRC)/*.c -o $@
