CC=clang
CFLAGS=-Wall -g -O0
RM=rm -f
OBJECTS=parse.o main.o walk.o query.o

xmlquery: $(OBJECTS)
	$(CC) $(CFLAGS) -o xmlquery $(OBJECTS)

.PHONY: clean
clean:
	$(RM) *.o
	$(RM) xmlquery
	$(RM) compile_commands.json
