#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

typedef enum{
  TK_RESERVED, //記号
  TK_IDENT,    //識別子
  TK_NUM,      //整数トークン
  TK_RETURN,   //return
  TK_IF,       //if
  TK_ELSE,     //else
  TK_WHILE,    //while
  TK_FOR,      //for
  TK_BLOCK,    //{
  TK_INT,      //int
  TK_EOF,      //入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token{
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum{
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_ASSIGN,
  ND_LVAR,
  ND_NUM,
  ND_RETURN,
  ND_IF,
  ND_ELSE,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_FUNC_CALL,
  ND_FUNC_DEF,
  ND_ADDR,
  ND_DEREF,
} NodeKind;

typedef struct Node Node;

struct Node{
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
  int offset;
  char *funcname;
};

typedef struct LVar LVar;

struct LVar{
  LVar *next;
  char *name;
  int len;
  int offset;
};

//関数ごとにローカル変数を用意
LVar *locals[20];

Token *token;

Node *code[100];

char *user_input;

int lavel_count;

int FuncDefCount;

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
bool consume(char *op);
Token *consume_kind(TokenKind kind);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
Token *tokenize();
Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
LVar *find_lvar(Token *tok);
LVar *find_arg(Token *tok);

// program    = stmt*
// stmt       = expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? primary
// primary    = num | ident | "(" expr ")"

void program();
Node *func();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen_lval(Node *node);
void gen(Node *node);