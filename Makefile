SRC := src

bin/slowjs: $(SRC)/*.c
	@rm -rf bin && mkdir bin
	$(CC) -I include $^ -o $@
