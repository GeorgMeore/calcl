#ifndef NODE_INCLUDED
#define NODE_INCLUDED

#include "arena.h"

typedef struct Node Node;

typedef enum {
	NumberNode,
	IdNode,
	ApplNode,
	SumNode,
	ProdNode,
	ExptNode,
	CmpNode,
	AndNode,
	OrNode,
	IfNode,
	FnNode,
	LetNode,
} NodeType;

typedef double NumberValue;

#define NumNode_value(nodeptr) ((nodeptr)->as.number)

typedef char *IdValue;

#define IdNode_value(nodeptr) ((nodeptr)->as.id)

typedef struct {
	Node *left;
	Node *right;
	int  op; // used by SumNode, ProdNode and CmpNode to store the operation
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
#define LetNode_name_value(nodeptr) IdNode_value(((nodeptr)->as.let.name))
#define LetNode_value(nodeptr) ((nodeptr)->as.let.value)

typedef union {
	NumberValue number; // NumberNode
	IdValue     id;     // IdNode
	IfValue     ifelse; // IfNode
	FnValue     fn;     // FnNode
	LetValue    let;    // LetNode
	PairValue   pair;   // others
} NodeValue;

struct Node {
	NodeType  type;
	NodeValue as;
};

Node *NumberNode_new(Arena *a, double number);
Node *IdNode_new(Arena *a, const char *string, int length);
Node *ApplicationNode_new(Arena *a, Node *left, Node *right);
Node *OpNode_new(Arena *a, Node *left, Node *right, NodeType type, int op);
Node *IfNode_new(Arena *a, Node *cond, Node *true, Node *false);
Node *FnNode_new(Arena *a, Node *param, Node *body);
Node *LetNode_new(Arena *a, Node *name, Node *value);
void Node_print(const Node *expr);
void Node_println(const Node *node);

#endif // NODE_INCLUDED
