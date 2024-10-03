#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "parse.h"
#include "query.h"
#include "walk.h"

#define _STRINGIFY1(X) #X
#define _STRINGIFY2(X) _STRINGIFY1(X)
#define TRY(CALL) \
	({ \
		errno = 0; \
		typeof(CALL) __ret = CALL; \
		if (errno) { \
			perror("TRY(" __FILE__ ":" _STRINGIFY2(__LINE__) ")"); \
			exit(1); \
		} \
		__ret; \
	})

int
main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage:\n\t%s <input.xml>\n", argv[0]);
		exit(1);
	}
	int f = TRY(open(argv[1], O_RDONLY));
	struct stat st;
	TRY(fstat(f, &st));
	size_t buflen = st.st_size + 1;
	char *buf = malloc(buflen);
	buf[buflen - 1] = '\0';
	read(f, buf, st.st_size);

	struct node *root;
	parse_document(buf, &root);
	if (root) {
		query_repl(root);
		put_document(root);
	} else {
		fprintf(stderr, "Parse error >:(\n");
	}

	free(buf);
}
