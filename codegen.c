#include "9cc.h"

LVar *find_lvar(Token *tok){
  for(LVar *var =locals[FuncDefCount]; var; var=var->next)
    if(var->len==tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

Node *new_node(NodeKind kind){
  Node *node=calloc(1, sizeof(Node));
  node->kind=kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs){
  Node *node =new_node(kind);
  node->lhs=lhs;
  node->rhs=rhs;
  return node;
}

Node *new_node_num(int val){
  Node *node=new_node(ND_NUM);
  node->val=val;
  return node;
}

// program    = func*
// func
//        "(" expr* ")"
//        "{" stmt* "}"
// stmt    = expr ";"
//        | "{" stmt* "}"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "while" "(" expr ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
// expr       = assign
// assign     = equality ("=" assign)?
// equality   = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary = "+"? primary
//       | "-"? primary
//       | "*" unary
//       | "&" unary
// primary    = num 
//            | ident ("(" stmt* ")")?
//            | "(" expr ")"

void program() {
  while(!at_eof()) {
    code[FuncDefCount++] = func();
  }
  code[FuncDefCount] = NULL;
}

Node *func() {
  Node *node = malloc(sizeof(Node)*100);
  int i = 0;
  Token *tok = consume_kind(TK_IDENT);
  if(tok == NULL)
    error("Not function");
  expect("(");
  while(!consume(")")) {
    Token *arg = consume_kind(TK_IDENT);
    LVar *lvar;
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals[FuncDefCount];
    lvar->name = arg->str;
    lvar->len = arg->len;
    if(locals[FuncDefCount])
      lvar->offset = locals[FuncDefCount]->offset+8;
    else 
      lvar->offset = 8;
    locals[FuncDefCount] = lvar;
    node->val++;
    if(consume(")"))
      break;
    else 
      expect(",");
  }
  consume_kind(TK_BLOCK);
  node->kind = ND_FUNC_DEF;
  node->funcname = tok->str;
  char *p = node->funcname;
  p[tok->len] = '\0';
  for(int j = 1; token; j++) {
    if(consume("}"))
      return node;
    node[j] = *stmt();
    node[0].offset++;
  }
  return node;
}

Node *stmt(){
    Node *node;
    if(consume_kind(TK_IF)) {
      expect("(");
      node = calloc(1, sizeof(Node));
      node->kind = ND_IF;
      node->lhs = expr();
      expect(")");
      node->rhs = stmt();
      if(consume_kind(TK_ELSE)){
        Node *els = calloc(1, sizeof(Node));
        els->kind = ND_ELSE;
        els->lhs = node->rhs;
        els->rhs = stmt();
        node->rhs = els;
      }
      return node;
    }

    if(consume_kind(TK_WHILE)){
      expect("(");
      node = calloc(1, sizeof(Node));
      node->kind = ND_WHILE;
      node->lhs = expr();
      expect(")");
      node->rhs = stmt();
      return node;
    }

    if(consume_kind(TK_FOR)){
      expect("(");
      node = calloc(1, sizeof(Node));
      node->kind = ND_FOR;
      Node *lef = calloc(1, sizeof(Node));
      lef->lhs = stmt();
      lef->rhs = stmt();
      node->lhs = lef;
      Node *rig = calloc(1, sizeof(Node));
      rig->lhs = expr();
      expect(")");
      rig->rhs = stmt();
      node->rhs = rig;
      return node;
    }

    if(consume_kind(TK_BLOCK)){
      // TODO メモリを随時確保できるようにする
      node = malloc(sizeof(Node)*100);
      node->kind = ND_BLOCK;
      for(int i = 1; token; i++){
        if(consume("}"))
          return node;
        node[i] = *stmt();
        node[0].offset++;
      }
    }
    
    if (consume_kind(TK_RETURN)) {
      node = calloc(1, sizeof(Node));
      node->kind = ND_RETURN;
      node->lhs = expr();
    } else {
      node = expr();
    }
    expect(";");
    return node;
}


Node *expr(){
  return assign();
}

Node *assign(){
    Node *node=equality();
    if(consume("="))
        node=new_binary(ND_ASSIGN, node, assign());
    return node;
}

Node *equality(){
  Node *node=relational();
  for(;;){
    if(consume("=="))
      node=new_binary(ND_EQ, node, relational());
    else if(consume("!="))
      node=new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational(){
  Node *node=add();
  for(;;){
    if(consume("<"))
      node=new_binary(ND_LT, node, add());
    else if(consume("<="))
      node=new_binary(ND_LE, node, add());
    else if(consume(">"))
      node=new_binary(ND_LT, add(), node);
    else if(consume(">="))
      node=new_binary(ND_LE, add(), node);
    else 
      return node;
  }
}

Node *add(){
  Node *node=mul();
  for(;;){
    if(consume("+"))
      node=new_binary(ND_ADD, node, mul());
    else if(consume("-"))
      node=new_binary(ND_SUB, node, mul());
    else 
      return node;
  }
}

Node *mul(){
  Node *node=unary();
  for(;;){
    if(consume("*"))
      node=new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node=new_binary(ND_DIV, node, unary());
    else 
      return node;
  }
}

Node *unary(){
  if(consume("+"))
    return unary();
  if(consume("-"))
    return new_binary(ND_SUB, new_node_num(0), unary());
  if(consume("*"))
    return new_binary(ND_DEREF, unary(), NULL);
  if(consume("&"))
    return new_binary(ND_ADDR, unary(), NULL);
  return primary();
}

Node *primary(){
  if(consume("(")){
    Node *node=expr();
    expect(")");
    return node;
  }
  Token *tok=consume_kind(TK_IDENT);
  int i = 0;
  if(tok){
    if(consume("(")){
      //関数呼び出し
      Node *node = malloc(sizeof(Node)*11);
      node->kind = ND_FUNC_CALL;
      node->funcname = tok->str;
      char *p = node->funcname;
      p[tok->len] = '\0';
      if(consume(")"))
        return node;
      // 引数　6個まで対応
       for(int i = 1; token; i++){
        node[i] = *expr();
        node[0].offset++;
        if(consume(")"))
          return node;
        expect(",");
      }
      return node;
    }
    Node *node=calloc(1, sizeof(Node));
    node->kind=ND_LVAR;
    
    LVar *lvar=find_lvar(tok);
    if(lvar){
      node->offset=lvar->offset;
    }else{
      lvar=calloc(1, sizeof(LVar));
      lvar->next=locals[FuncDefCount];
      lvar->name=tok->str;
      lvar->len=tok->len;
      if(locals[FuncDefCount])
        lvar->offset=locals[FuncDefCount]->offset+8;
      else 
        lvar->offset=8;
      node->offset=lvar->offset;
      locals[FuncDefCount]=lvar;
    }
    return node;
  }

  return new_node_num(expect_number());
}

void gen_lval(Node *node){
    if(node->kind!=ND_LVAR)
        error("代入の左辺値が変数ではありません");

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

void gen(Node *node){
  char arg[][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
  switch(node->kind){
    case ND_ADDR:
        printf("    push %d\n", node->lhs->offset);
      return;
    case ND_DEREF:
      gen_lval(node->lhs);
      printf("    pop rax\n");
      printf("    mov rax, [rax]\n");
      printf("    push rax\n");
      printf("    push rbp\n");
      printf("    pop rax\n");
      printf("    pop rbx\n");
      printf("    sub rax, rbx\n");
      printf("    mov rax, [rax]\n");
      printf("    push rax\n");
      return;
    case ND_FUNC_DEF:
      printf("%s:\n", node->funcname);
      printf("    push rbp\n");
      printf("    mov rbp, rsp\n");
      printf("    sub rsp, %d\n", 200);
      for(int i = 0; i < node->val; i++) {
        printf("    mov rax, rbp\n");
        printf("    sub rax, %d\n", (i+1)*8);
        printf("    push %s\n", arg[i]);
        printf("    pop rbx\n");
        printf("    mov [rax], rbx\n");
      }
      for(int i = 1; i <= node[0].offset; i++) {
        gen(&node[i]);
      }
      return;
    case ND_FUNC_CALL:
      for(int i = 0; i < node->offset; i++)
        gen(&node[node->offset - i]);
      for(int i = 0; i < node->offset; i++)
        printf("    pop %s\n", arg[i]);
      printf("    mov rax, rsp\n");
      printf("    and rax, 15\n");
      printf("    jnz .Lend%03d\n", lavel_count);
      lavel_count++;
      printf("    mov rax, 0\n");
      printf("    call %s\n", node->funcname);
      printf("    jmp .Lend%03d\n", lavel_count);
      lavel_count--;
      printf(".Lend%03d:\n", lavel_count);
      printf("    sub rsp, 8\n");
      printf("    mov rax, 0\n");
      printf("    call %s\n", node->funcname);
      printf("    add rsp, 8\n");
      lavel_count++;
      printf(".Lend%03d:\n", lavel_count);
      lavel_count++;
      printf("    push rax\n");
      return;
    case ND_BLOCK:
      for(int i = 1; i <= node[0].offset; i++) {
        gen(&node[i]);
        printf("    pop rax\n");
      }
      return;
    case ND_FOR:
      gen(node->lhs->lhs);
      printf(".Lend%03d:\n", lavel_count++);
      gen(node->lhs->rhs);
      printf("    pop rax\n");
      printf("    cmp rax, 0\n");
      printf("    je .Lend%03d\n", lavel_count--);
      gen(node->rhs->rhs);
      gen(node->rhs->lhs);
      printf("    jmp .Lend%03d\n", lavel_count++);
      printf(".Lend%03d:\n", lavel_count);
      lavel_count++;
      return;
    case ND_WHILE:
      printf(".Lend%03d:\n", lavel_count++);
      gen(node->lhs);
      printf("    pop rax\n");
      printf("    cmp rax, 0\n");
      printf("    je .Lend%03d\n", lavel_count--);
      gen(node->rhs);
      printf("    jmp .Lend%03d\n", lavel_count++);
      printf(".Lend%03d:\n", lavel_count);
      lavel_count++;
      return;
    case ND_IF:
      gen(node->lhs);
      printf("    pop rax\n");
      printf("    cmp rax, 0\n");
      printf("    je .Lend%03d\n", lavel_count++);
      if(node->rhs->kind == ND_ELSE)
        gen(node->rhs->lhs);
      else
        gen(node->rhs);
      printf("    je .Lend%03d\n", lavel_count--);
      printf(".Lend%03d:\n", lavel_count++);
      if(node->rhs->kind == ND_ELSE)
        gen(node->rhs->rhs);
      printf(".Lend%03d:\n", lavel_count);
      lavel_count++;
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("    pop rax\n");
      printf("    mov rsp, rbp\n");
      printf("    pop rbp\n");
      printf("    ret\n");
      return;
    case ND_NUM:
      printf("    push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      printf("    pop rax\n");
      printf("    mov rax, [rax]\n");
      printf("    push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);
      printf("    pop rdi\n");
      printf("    pop rax\n");
      printf("    mov [rax], rdi\n");
      printf("    push rdi\n");
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("    pop rdi\n");
  printf("    pop rax\n");

  switch(node->kind){
    case ND_ADD:
      printf("    add rax, rdi\n");
      break;      
    case ND_SUB:
      printf("    sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("    imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("    cqo\n");
      printf("    idiv rdi\n");
      break;
    case ND_EQ:
      printf("    cmp rax, rdi\n");
      printf("    sete al\n");
      printf("    movzb rax, al\n");
      break;
    case ND_NE:
      printf("    cmp rax, rdi\n");
      printf("    setne al\n");
      printf("    movzb rax, al\n");
      break;
    case ND_LT:
      printf("    cmp rax, rdi\n");
      printf("    setl al\n");
      printf("    movzb rax, al\n");
      break;
    case ND_LE:
      printf("    cmp rax, rdi\n");
      printf("    setle al\n");
      printf("    movzb rax, al\n");
      break;
  }

  printf("    push rax\n");
}