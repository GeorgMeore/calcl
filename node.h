#ifndef NODE_INCLUDED
#define NODE_INCLUDED

typedef struct Node Node;

typedef enum {
	NUMBER_NODE,
	ID_NODE,
	APPLICATION_NODE,
	SUM_NODE,
	PRODUCT_NODE,
	EXPT_NODE,
	CMP_NODE,
	AND_NODE,
	OR_NODE,
	IF_NODE,
} NodeType;

typedef double NumberValue;

typedef char *IdValue;

typedef struct {
	Node *left;
	Node *right;
} PairValue;

typedef struct {
	Node *cond;
	Node *true;
	Node *false;
} IfValue;

typedef union {
	NumberValue number; // NUMBER_NODE
	IdValue id;         // ID_NODE
	IfValue ifelse;     // IF_NODE
	PairValue pair;     // others
} NodeValue;

struct Node {
	NodeType type;
	NodeValue value;
};

Node *NumberNode_new(const char *string, int length);
Node *IdNode_new(const char *string, int length);
Node *ApplicationNode_new(Node *left, Node *right);
Node *SumNode_new(Node *left, Node *right);
Node *ProductNode_new(Node *left, Node *right);
Node *ExptNode_new(Node *base, Node *exponent);
Node *CmpNode_new(Node *left, Node *right);
Node *AndNode_new(Node *left, Node *right);
Node *OrNode_new(Node *left, Node *right);
Node *IfNode_new(Node *cond, Node *true, Node *false);
void Node_drop(Node *node);

#endif // NODE_INCLUDED
