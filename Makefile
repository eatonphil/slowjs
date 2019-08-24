SRC := src

bin/slowjs: $(SRC)/*.c
	$(CC) -I include $^ -o $@
