#include "codegen.h"

#include <stdio.h>

#include "node.h"
#include "object.h"
#include "values.h"
#include "env.h"
#include "error.h"
#include "gc.h"
#include "opts.h"


// TODO: only do type assertions where necessary
// TODO: only save registers when they need to be saved
// TODO: lexical addressing

#define ERROR_PREFIX "compillation error"

#define REG_VAL  "%r12"
#define REG_ENV  "%r13"
#define REG_LINK "%r14"
#define REG_TMP  "%r15"

typedef enum {
	LinkNext,
	LinkReturn
} Linkage;

static int generate_id(void)
{
	static int id = 0;
	id += 1;
	return id;
}

static int forceable(const Node *expr)
{
	if (!lazy)
		return 0;
	switch (expr->type) {
		case IdNode:
			return 1;
		case ApplNode:
			return 1;
		case IfNode:
			return forceable(IfNode_true(expr)) || forceable(IfNode_false(expr));
		case ExptNode:
		case ProdNode:
		case SumNode:
		case CmpNode:
		case AndNode:
		case OrNode:
			return forceable(PairNode_left(expr)) || forceable(PairNode_right(expr));
		case LetNode:
		case NumberNode:
		case FnNode:
	}
	return 0;
}

static void compile_dispatch(const Node *expr, Linkage l);

static void compile_gc_call(void)
{
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	mov %s, %%rsi\n", REG_ENV);
	printf("	mov %%rsp, %%rdx\n");
	printf("	mov %%rbp, %%rcx\n");
	printf("	call GC_collect_comp\n");
}

static void compile_stack_push(PtrType type, const char *reg)
{
	printf("	push %s\n", reg);
	printf("	pushq $%d\n", type);
}

static void compile_stack_pop(const char *reg)
{
	printf("	add $8, %%rsp\n");
	printf("	pop %s\n", reg);
}

static void compile_ret(void)
{
	compile_stack_pop(REG_LINK);
	printf("	jmp *%s\n", REG_LINK);
}

static void compile_force_sub(void)
{
	printf("force:\n");
	printf("	cmpl $%d, (%s)\n", CompthunkObject, REG_VAL);
	printf("	jne force_ret\n");
	printf("	cmpq $0, %d(%s)\n", ObjFldOff(CompThunk, value), REG_VAL);
	printf("	jne force_get_value\n");
	compile_stack_push(PTR_ADDR, REG_LINK);
	compile_stack_push(PTR_OBJ, REG_VAL);
	printf("	lea force_recurse(%%rip), %s\n", REG_LINK);
	printf("	mov %d(%s), %s\n", ObjFldOff(CompThunk, env), REG_VAL, REG_ENV);
	printf("	jmp *%d(%s)\n", ObjFldOff(CompThunk, text), REG_VAL);
	printf("force_recurse:\n");
	printf("	lea force_computed(%%rip), %s\n", REG_LINK);
	printf("	jmp force\n");
	printf("force_computed:\n");
	compile_stack_pop(REG_TMP);
	compile_stack_pop(REG_LINK);
	printf("	movq %s, %d(%s)\n", REG_VAL, ObjFldOff(CompThunk, value), REG_TMP);
	printf("	jmp force_ret\n");
	printf("force_get_value:\n");
	printf("	mov %d(%s), %s\n", ObjFldOff(CompThunk, value), REG_VAL, REG_VAL);
	printf("force_ret:\n");
	printf("	jmp *%s\n", REG_LINK);
}

static void compile_force_call(void)
{
	int id = generate_id();
	printf("	cmpl $%d, (%s)\n", CompthunkObject, REG_VAL);
	printf("	jne force_end%d\n", id);
	compile_stack_push(PTR_OBJ, REG_ENV);
	printf("	lea force_done%d(%%rip), %s\n", id, REG_LINK);
	printf("	jmp force\n");
	printf("force_done%d:\n", id);
	compile_stack_pop(REG_ENV);
	printf("force_end%d:\n", id);
}

static void compile_type_assertion(ObjectType type)
{
	printf("	cmpl $%d, (%s)\n", type, REG_VAL);
	printf("	jne failure\n");
}

static void compile_num(const Node *expr)
{
	int id = generate_id();
	printf(".data\n");
	printf("v%d: .double %lf\n", id, NumNode_value(expr));
	printf(".text\n");
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	movsd v%d(%%rip), %%xmm0\n", id);
	printf("	call GC_alloc_number\n");
	printf("	mov %%rax, %s\n", REG_VAL);
}

static void compile_id(const Node *expr)
{
	int id = generate_id();
	printf(".data\n");
	printf("i%d: .asciz \"%s\"\n", id, IdNode_value(expr));
	printf(".text\n");
	printf("	lea %d(%s), %%rdi\n", ObjValOff(Env), REG_ENV);
	printf("	lea i%d(%%rip), %%rsi\n", id);
	printf("	call Env_get\n");
	printf("	mov %%rax, %s\n", REG_VAL);
}

static void compile_if(const Node *expr, Linkage l)
{
	int id = generate_id();
	compile_dispatch(IfNode_cond(expr), LinkNext);
	if (forceable(IfNode_cond(expr))) {
		compile_force_call();
	}
	if (!typed) {
		compile_type_assertion(NumObject);
	}
	printf("	cmpq $0, %d(%s)\n", ObjFldOff(Num, num), REG_VAL);
	printf("	je false_branch%d\n", id);
	printf("true_branch%d:\n", id);
	compile_dispatch(IfNode_true(expr), l);
	printf("	jmp if_end%d\n", id);
	printf("false_branch%d:\n", id);
	compile_dispatch(IfNode_false(expr), l);
	printf("if_end%d:\n", id);
}

static void compile_fn(const Node *expr)
{
	int id = generate_id();
	printf(".data\n");
	printf("	a%d: .asciz \"%s\"\n", id, FnNode_param_value(expr));
	printf(".text\n");
	printf("	jmp fn_end%d\n", id);
	printf("fn%d:\n", id);
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	mov %s, %%rsi\n", REG_ENV);
	printf("	call GC_alloc_env\n");
	printf("	mov %%rax, %s\n", REG_ENV);
	printf("	lea %d(%%rax), %%rdi\n", ObjValOff(Env));
	printf("	lea a%d(%%rip), %%rsi\n", id);
	printf("	mov %s, %%rdx\n", REG_VAL);
	printf("	call Env_add\n");
	compile_gc_call();
	compile_stack_push(PTR_ADDR, REG_LINK);
	compile_dispatch(FnNode_body(expr), LinkReturn);
	printf("fn_end%d:\n", id);
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	mov %s, %%rsi\n", REG_ENV);
	printf("	lea fn%d(%%rip), %%rdx\n", id);
	printf("	call GC_alloc_compfn\n");
	printf("	mov %%rax, %s\n", REG_VAL);
}

static void compile_application(const Node *expr, Linkage l)
{
	int id = generate_id();
	compile_dispatch(PairNode_left(expr), LinkNext);
	if (forceable(PairNode_left(expr))) {
		compile_force_call();
	}
	if (!typed) {
		compile_type_assertion(CompfnObject);
	}
	compile_stack_push(PTR_OBJ, REG_VAL);
	if (lazy) {
		printf("	jmp thunk_end%d\n", id);
		printf("thunk%d:\n", id);
		compile_stack_push(PTR_ADDR, REG_LINK);
		compile_gc_call();
		compile_dispatch(PairNode_right(expr), LinkReturn);
		printf("thunk_end%d:\n", id);
		printf("	mov gc(%%rip), %%rdi\n");
		printf("	mov %s, %%rsi\n", REG_ENV);
		printf("	lea thunk%d(%%rip), %%rdx\n", id);
		printf("	call GC_alloc_compthunk\n");
		printf("	mov %%rax, %s\n", REG_VAL);
	} else {
		compile_dispatch(PairNode_right(expr), LinkNext);
	}
	compile_stack_pop(REG_TMP);
	if (l == LinkNext) {
		compile_stack_push(PTR_OBJ, REG_ENV);
	}
	printf("	mov %d(%s), %s\n", ObjFldOff(CompFn, env), REG_TMP, REG_ENV);
	if (l == LinkNext) {
		printf("	lea after_call%d(%%rip), %s\n", id, REG_LINK);
		printf("	jmp *%d(%s)\n", ObjFldOff(CompFn, text), REG_TMP);
		printf("after_call%d:\n", id);
		compile_stack_pop(REG_ENV);
	} else {
		compile_stack_pop(REG_LINK);
		printf("	jmp *%d(%s)\n", ObjFldOff(CompFn, text), REG_TMP);
	}
}

static void compile_let(const Node *expr)
{
	int id = generate_id();
	printf(".data\n");
	printf("i%d: .asciz \"%s\"\n", id, LetNode_name_value(expr));
	printf(".text\n");
	compile_dispatch(LetNode_value(expr), LinkNext);
	printf("	lea %d(%s), %%rdi\n", ObjValOff(Env), REG_ENV);
	printf("	lea i%d(%%rip), %%rsi\n", id);
	printf("	mov %s, %%rdx\n", REG_VAL);
	printf("	call Env_add\n");
}

static void compile_cmp_pair(int op)
{
	int id = generate_id();
	printf("	comisd %%xmm1, %%xmm0\n");
	printf("	jp cmp_false%d\n", id); /* NAN */
	switch (op) {
		case '>':
			printf("	jbe cmp_false%d\n", id);
			break;
		case '<':
			printf("	jae cmp_false%d\n", id);
			break;
		case '=':
			printf("	jne cmp_false%d\n", id);
			break;
	}
	printf("	movq true(%%rip), %%xmm0\n");
	printf("	jmp cmp_end%d\n", id);
	printf("cmp_false%d:\n", id);
	printf("	movq false(%%rip), %%xmm0\n");
	printf("cmp_end%d:\n", id);
}

static void compile_pair(const Node *expr)
{
	int op = PairNode_op(expr);
	compile_dispatch(PairNode_left(expr), LinkNext);
	if (forceable(PairNode_left(expr))) {
		compile_force_call();
	}
	if (!typed) {
		compile_type_assertion(NumObject);
	}
	compile_stack_push(PTR_OBJ, REG_VAL);
	compile_dispatch(PairNode_right(expr), LinkNext);
	if (forceable(PairNode_right(expr))) {
		compile_force_call();
	}
	if (!typed) {
		compile_type_assertion(NumObject);
	}
	compile_stack_pop(REG_TMP);
	printf("	movq %d(%s), %%xmm0\n", ObjFldOff(Num, num), REG_TMP);
	printf("	movq %d(%s), %%xmm1\n", ObjFldOff(Num, num), REG_VAL);
	switch (op) {
		case '^':
			printf("	call pow\n");
			break;
		case '*':
			printf("	mulsd %%xmm1, %%xmm0\n");
			break;
		case '/':
			printf("	divsd %%xmm1, %%xmm0\n");
			break;
		case '%':
			printf("	call fmod\n");
			break;
		case '+':
			printf("	addsd %%xmm1, %%xmm0\n");
			break;
		case '-':
			printf("	subsd %%xmm1, %%xmm0\n");
			break;
		case '>':
		case '<':
		case '=':
			compile_cmp_pair(op);
			break;
		default:
			errorf("unknown binary operation: '%c'", op);
			return;
	}
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	call GC_alloc_number\n");
	printf("	mov %%rax, %s\n", REG_VAL);
}

static void compile_and(const Node *expr, Linkage l)
{
	int id = generate_id();
	compile_dispatch(PairNode_left(expr), LinkNext);
	if (forceable(PairNode_left(expr))) {
		compile_force_call();
	}
	if (!typed) {
		compile_type_assertion(NumObject);
	}
	printf("	cmpq $0, %d(%s)\n", ObjFldOff(Num, num), REG_VAL);
	printf("	je and_false%d\n", id);
	compile_dispatch(PairNode_right(expr), l);
	printf("and_false%d:\n", id);
	if (l == LinkReturn) {
		compile_ret();
	}
}

static void compile_or(const Node *expr, Linkage l)
{
	int id = generate_id();
	compile_dispatch(PairNode_left(expr), LinkNext);
	if (forceable(PairNode_left(expr))) {
		compile_force_call();
	}
	if (!typed) {
		compile_type_assertion(NumObject);
	}
	printf("	cmpq $0, %d(%s)\n", ObjFldOff(Num, num), REG_VAL);
	printf("	jne or_true%d\n", id); // ????
	compile_dispatch(PairNode_right(expr), l);
	printf("or_true%d:\n", id);
	if (l == LinkReturn) {
		compile_ret();
	}
}

static void compile_dispatch(const Node *expr, Linkage l)
{
	printf("// "); Node_println(expr);
	switch (expr->type) {
		case NumberNode:
			compile_num(expr);
			break;
		case FnNode:
			compile_fn(expr);
			break;
		case IdNode:
			compile_id(expr);
			break;
		case ExptNode:
		case ProdNode:
		case SumNode:
		case CmpNode:
			compile_pair(expr);
			break;
		case AndNode:
			return compile_and(expr, l);
		case OrNode:
			return compile_or(expr, l);
		case IfNode:
			return compile_if(expr, l);
		case ApplNode:
			return compile_application(expr, l);
		case LetNode:
			compile_let(expr);
			break;
	}
	// NOTE: AndNode, OrNode, IfNode and ApplNode
	// must handle linkage themselves
	if (l == LinkReturn) {
		compile_ret();
	}
}

void compile(const Node *expr)
{
	compile_dispatch(expr, LinkNext);
	if (forceable(expr)) {
		compile_force_call();
	}
	if (expr->type != LetNode) {
		printf("	mov %s, %%rdi\n", REG_VAL);
		printf("	call Object_println\n");
	}
	compile_gc_call();
}

void compile_begin(void)
{
	printf(".global main\n");
	printf(".data\n");
	printf("gc: .quad 0\n");
	printf("env: .quad 0\n");
	printf("true:  .double 1.0\n");
	printf("false: .double 0.0\n");
	printf(".text\n");
	compile_force_sub();
	printf("main:\n");
	printf("	push %%rbp\n");
	printf("	mov %%rsp, %%rbp\n");
	printf("	call GC_new\n");
	printf("	mov %%rax, gc(%%rip)\n");
	printf("	mov %%rax, %%rdi\n");
	printf("	mov $0, %%rsi\n");
	printf("	call GC_alloc_env\n");
	printf("	mov %%rax, env(%%rip)\n");
	printf("	mov env(%%rip), %s\n", REG_ENV);
}

void compile_end(void)
{
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	mov $0, %%rsi\n");
	printf("	mov $0, %%rdx\n");
	printf("	call GC_collect\n");
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	call GC_drop\n");
	printf("	mov $0, %%rax\n");
	printf("	pop %%rbp\n");
	printf("	ret\n");
	// TODO: log something maybe?
	printf("failure:\n");
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	mov $0, %%rsi\n");
	printf("	mov $0, %%rdx\n");
	printf("	call GC_collect\n");
	printf("	mov gc(%%rip), %%rdi\n");
	printf("	call GC_drop\n");
	printf("	mov %%rbp, %%rsp\n");
	printf("	mov $1, %%rax\n");
	printf("	pop %%rbp\n");
	printf("	ret\n");
}
