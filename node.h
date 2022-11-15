#ifndef NODE_INCLUDED
#define NODE_INCLUDED

typedef struct Node Node;

typedef enum {
	NUMBER_NODE,
	SUM_NODE,
	PRODUCT_NODE,
} NodeType;

typedef int NumberValue;

typedef struct {
	Node *left;
	Node *right;
} PairValue;

typedef union {
	// NUMBER_NODE
	NumberValue number;
	// SUM_NODE | PRODUCT_NODE
	PairValue pair;
} NodeValue;

struct Node {
	NodeType type;
	NodeValue value;
};

Node *NumberNode_new(const char *string, int length);
Node *SumNode_new(Node *left, Node *right);
Node *ProductNode_new(Node *left, Node *right);

#endif // NODE_INCLUDED
