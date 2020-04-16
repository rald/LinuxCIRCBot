CC=gcc
OBJS=bot.o
EXE=bot
$(EXE): $(OBJS)
bot.o: bot.c
clean:
	rm $(OBJS) $(EXE)
