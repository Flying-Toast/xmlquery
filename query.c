#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "query.h"

struct query {
	char *tagname;
	struct attrlist *attrs;
	struct query *child_query;
};

static void
put_query(struct query *q) {
	if (!q)
		return;
	put_query(q->child_query);
	put_attrlist(q->attrs);
	if (q->tagname)
		free(q->tagname);
	free(q);
}

static struct query *
parse_querypart(char *s, char **rest) {
	if (!*s)
		return NULL;
	struct query *ret = malloc(sizeof(*ret));
	ret->child_query = NULL;
	ret->attrs = NULL;
	ret->tagname = NULL;

	eatsp(&s);
	ret->tagname = parse_tagname(s, rest);
	if (ret->tagname)
		s = *rest;
	eatsp(&s);
	if (*s && *s == '[') {
		s++;
		if (parse_attrlist(s, &ret->attrs, rest) == -1) {
			fputs("\nFailed to parse attrlist for query\n", stderr);
			ret->attrs = NULL;
			goto retnull_putret;
		}
		s = *rest;
		eatsp(&s);
		if (!*s || *(s++) != ']') {
			fputs("\nExpected ']' in query\n", stdout);
			goto retnull_putret;
		}
	}


	*rest = s;
	return ret;
retnull_putret:
	put_query(ret);
	return NULL;
}

static struct query *
do_parse_querypart(char *s, char **rest) {
	struct query *ret = parse_querypart(s, rest);
	if (!ret)
		fprintf(stderr, "\nparse_querypart failed on s=%s\n", s);

	return ret;
}


static struct query *
get_query(void) {
	fputs("query> ", stdout);
	fflush(stdout);
	char qbuf[10000];
	if (!fgets(qbuf, sizeof(qbuf), stdin)) {
		puts("\nSee ya later, alligator.");
		exit(0);
	}
	char *s = NULL;
	if (qbuf[0] == '\n') {
		puts("You didn't enter anything >:(");
		return NULL;
	}
	struct query *head = do_parse_querypart(qbuf, &s);
	if (!head)
		return NULL;

	struct query **next = &head->child_query;
	for (;;) {
		*next = NULL;
		eatsp(&s);
		if (!*s)
			break;
		if (*s == '|') {
			s++;
		} else {
			if (*s)
				fputs("\nExpected end of string or '|'\n", stderr);
			break;
		}
		eatsp(&s);
		char *rest = NULL;
		struct query *childq = do_parse_querypart(s, &rest);
		s = rest;
		if (!childq)
			break;
		*next = childq;
		next = &childq->child_query;
	}

	return head;
}

static int
matches(struct query *q, struct node *node) {
	if (node->kind != NODE_ELT) {
		fputs("\ntried to match on non-elt node\n", stderr);
		return 0;
	}

	if (!strcasecmp(q->tagname, node->elt.tagname)) {
		for (struct attrlist *l = q->attrs; l; l = l->next) {
			if (!attrlist_has(node->elt.attrs, &l->attr))
				return 0;
		}
		return 1;
	}

	return 0;
}

static void
eval(struct query *q, struct node *root) {
	if (!root) {
		fputs("\nTried to query a NULL node\n", stderr);
		return;
	}

	if (!q)
		return;

	if (root->kind != NODE_ELT) {
		fputs("\nTried to eval on non-elt node\n", stderr);
		return;
	}

	if (root->kind == NODE_ELT && matches(q, root)) {
		if (q->child_query == NULL) {
			puts("\n\n\n===== Query match found: =====");
			print_node(root);
		} else {
			for (struct nodelist *chld = root->elt.children; chld; chld = chld->next) {
				if (chld->node.kind == NODE_ELT)
					eval(q->child_query, &chld->node);
			}
		}
	}
}

void
query_repl(struct node *root) {
	for (;;) {
		struct query *q = get_query();
		if (!q) {
			fputs("Invalid query >:(\n", stderr);
			continue;
		}
		eval(q, root);
		put_query(q);
	}
}
