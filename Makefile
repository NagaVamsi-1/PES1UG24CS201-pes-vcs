CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lcrypto -lz

# All source files
SRCS = pes.c object.c tree.c index.c commit.c
OBJS = $(SRCS:.c=.o)

# ─── Main program (Phase 3) ───
pes: $(OBJS)
	$(CC) -o pes $(OBJS) $(LDFLAGS)

# ─── Phase 1 test ───
test_objects: test_objects.o object.o
	$(CC) -o test_objects test_objects.o object.o $(LDFLAGS)

# ─── Phase 2 test ───
test_tree: test_tree.o tree.o object.o index.o
	$(CC) -o test_tree test_tree.o tree.o object.o index.o $(LDFLAGS)

test-integration: pes
	./pes init
	echo "hello" > file1.txt
	./pes add file1.txt
	./pes commit -m "test"
	./pes log
# Compile rule
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Clean
clean:
	rm -f *.o pes test_objects test_tree
	rm -rf .pes

.PHONY: clean
