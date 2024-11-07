#ifndef __HAVE_PARSE_H
#define __HAVE_PARSE_H

enum node_kind {
	NODE_ELT,
	NODE_TEXT,
	NODE_COMMENT,
	NODE_WHITESPACE,
};

struct attr {
	char *name;
	char *val;
};

struct attrlist {
	struct attr attr;
	struct attrlist *next;
};


struct node {
	enum node_kind kind;
	union {
		struct {
			char *tagname;
			struct attrlist *attrs;
			struct nodelist *children;
		} elt;

		struct {
			char *content;
		} text;

		struct {
			char *content;
		} comment;
	};
};

struct nodelist {
	struct node node;
	struct nodelist *next;
};

void print_node(struct node *n);
int parse_document(char *src, struct node **out);
void put_document(struct node *root);
void put_attrlist(struct attrlist *l);
int parse_attrlist(char *s, struct attrlist **out, char **rest);
void eatsp(char **s);
char *parse_tagname(char *s, char **rest);
int attrlist_has(struct attrlist *l, struct attr *att);
void print_attrlist(struct attrlist *al, char *indentstr);

#endif
