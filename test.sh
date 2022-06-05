#!/bin/bash

assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cd func
    cc -c func.c
    cd ..
    cc -o tmp tmp.s func/func.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected, but got $actual"
        exit 1
    fi
}

# assert 0 "0;"
# assert 42 "42;"
# assert 21 '5+20-4;'
# assert 41 ' 12 + 34 - 5; '
# assert 47 '5+6*7;'
# assert 15 '5*(9-6);'
# assert 4 '(3+5)/2;'
# assert 10 '-10+20;'
# assert 10 '- -10;'
# assert 10 '- - +10;'
# assert 0 '0==1;'
# assert 1 '42==42;'
# assert 1 '0!=1;'
# assert 0 '42!=42;'

# assert 1 '0<1;'
# assert 0 '1<1;'
# assert 0 '2<1;'
# assert 1 '0<=1;'
# assert 1 '1<=1;'
# assert 0 '2<=1;'

# assert 1 '1>0;'
# assert 0 '1>1;'
# assert 0 '1>2;'
# assert 1 '1>=0;'
# assert 1 '1>=1;'
# assert 0 '1>=2;'

# assert 14 "a=3;
# b=5*6-8;
# a+b/2;"
# assert 6 "foo=1;
# bar=2+3;
# return foo+bar;"

# assert 5 "return 5;"

# assert 1 "if(1) return 1; return 3;"

# assert 3 "if(0) return 1; else return 3;"

# assert 0 "a = 30; while(a > 0) a = a-1; return a;"

# assert 55 "sum = 0; for(i=1; i<=10; i=i+1) sum = sum+i; return sum;"

# assert 30 "
# a = 0;
# i = 0;
# while(a < 10){
#     i = i+2;
#     a = a+1;
# }

# return a + i;
# "

# assert 0 "foo(); return 0;"
# assert 0 "bar(3, 4); return 0;"
# assert 0 "bar2(3, 4, 5); return 0;"

assert 120 "
int fact(int n)
{
  int m;
  m = 0;

  if (n == 0)
    return 1;
  m = fact(n - 1);
  return n * m;
}

int main()
{
  int ans;
  ans = fact(5);
  return ans;
}
"
assert 3 "
int main(){
    int x; 
    x = 3;
    int y;
    y = &x;
    return *y;
}"

assert 3 "
int main() {
    int x; 
    x = 0;
    int y; 
    y = 3;
    int p; 
    p = &y;
    return *p;
}"
assert 3 "int main() { int a; a = 3; return a;}"
echo OK