#include "9cc.h"

int main(int argc, char **argv){
  if(argc!=2){
    fprintf(stderr, "invalid args\n");
    return 1;
  }

  locals[0] = NULL;
  user_input = argv[1];
  token = tokenize();
  program();
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  for(int i = 0; code[i]; i++) {
    gen(code[i]);
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
  }
  return 0;
}