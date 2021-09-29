
typedef int (*func)(int a, int b);

int compute(func f, int a, int b) {
    return f(a, b);
}

int add(int a, int b)
{
    return a + b;
}

int foo() {
    return compute(add, 2, 4);
}