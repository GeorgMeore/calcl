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
	FN_NODE,
	LET_NODE,
} NodeType;

typedef double NumberValue;

typedef char *IdValue;

typedef struct {
	Node *left;
	Node *right;
	int op; // used by SUM_NODE, PRODUCT_NODE and CMP_NODE to store the operation
} PairValue;

typedef struct {
	Node *cond;
	Node *true;
	Node *false;
} IfValue;

typedef struct {
	Node *param;
	Node *body;
} FnValue;

typedef struct {
	Node *name;
	Node *value;
} LetValue;

typedef union {
	NumberValue number; // NUMBER_NODE
	IdValue id;         // ID_NODE
	IfValue ifelse;     // IF_NODE
	FnValue fn;         // FN_NODE
	LetValue let;       // LET_NODE
	PairValue pair;     // others
} NodeValue;

struct Node {
	NodeType type;
	NodeValue as;
};

Node *NumberNode_new(const char *string, int length);
Node *IdNode_new(const char *string, int length);
Node *ApplicationNode_new(Node *left, Node *right);
Node *SumNode_new(Node *left, Node *right, int op);
Node *ProductNode_new(Node *left, Node *right, int op);
Node *ExptNode_new(Node *base, Node *exponent);
Node *CmpNode_new(Node *left, Node *right, int op);
Node *AndNode_new(Node *left, Node *right);
Node *OrNode_new(Node *left, Node *right);
Node *IfNode_new(Node *cond, Node *true, Node *false);
Node *FnNode_new(Node *param, Node *body);
Node *LetNode_new(Node *name, Node *value);
void Node_drop(Node *node);
Node *Node_copy(const Node *node);

#endif // NODE_INCLUDED
