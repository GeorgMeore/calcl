#ifndef NODE_INCLUDED
#define NODE_INCLUDED

typedef struct Node Node;

typedef enum {
	NUMBER_NODE,
	ID_NODE,
	NEG_NODE,
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

#define NumNode_value(nodeptr) ((nodeptr)->as.number)

typedef char *IdValue;

#define IdNode_value(nodeptr) ((nodeptr)->as.id)

typedef Node *NegValue;

#define NegNode_value(nodeptr) ((nodeptr)->as.neg)

typedef struct {
	Node *left;
	Node *right;
	int  op; // used by SUM_NODE, PRODUCT_NODE and CMP_NODE to store the operation
} PairValue;

#define PairNode_left(nodeptr) ((nodeptr)->as.pair.left)
#define PairNode_right(nodeptr) ((nodeptr)->as.pair.right)
#define PairNode_op(nodeptr) ((nodeptr)->as.pair.op)

typedef struct {
	Node *cond;
	Node *true;
	Node *false;
} IfValue;

#define IfNode_cond(nodeptr) ((nodeptr)->as.ifelse.cond)
#define IfNode_true(nodeptr) ((nodeptr)->as.ifelse.true)
#define IfNode_false(nodeptr) ((nodeptr)->as.ifelse.false)

typedef struct {
	Node *param;
	Node *body;
} FnValue;

#define FnNode_param(nodeptr) ((nodeptr)->as.fn.param)
#define FnNode_param_value(nodeptr) IdNode_value(((nodeptr)->as.fn.param))
#define FnNode_body(nodeptr) ((nodeptr)->as.fn.body)

typedef struct {
	Node *name;
	Node *value;
} LetValue;

#define LetNode_name(nodeptr) ((nodeptr)->as.let.name)
#define LetNode_value(nodeptr) ((nodeptr)->as.let.value)

typedef union {
	NumberValue number; // NUMBER_NODE
	IdValue     id;     // ID_NODE
	NegValue    neg;    // NEG_NODE
	IfValue     ifelse; // IF_NODE
	FnValue     fn;     // FN_NODE
	LetValue    let;    // LET_NODE
	PairValue   pair;   // others
} NodeValue;

struct Node {
	NodeType  type;
	NodeValue as;
};

Node *NumberNode_new(const char *string, int length);
Node *IdNode_new(const char *string, int length);
Node *NegNode_new(Node *value);
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
