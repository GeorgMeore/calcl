#ifndef NODE_INCLUDED
#define NODE_INCLUDED

typedef struct Node Node;
typedef union NodeValue NodeValue;
typedef enum NodeType NodeType;

enum NodeType {
	NUMBER_NODE,
	SUM_NODE,
	PRODUCT_NODE,
};

union NodeValue {
	/* NUMBER_NODE */
	int number;
	/* SUM_NODE | PRODUCT_NODE */
	struct {
		Node *left;
		Node *right;
	};
};

struct Node {
	NodeType type;
	NodeValue value;
};

Node *NumberNode_new(int value);
Node *SumNode_new(Node *left, Node *right);
Node *ProductNode_new(Node *left, Node *right);

#endif // NODE_INCLUDED
