SRC := src

bin/slowjs: $(SRC)/main.c $(SRC)/*.c
	$(CC) -I include $< -o $@
