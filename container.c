#include "9cc.h"

void expect(int line, int expected, int actual) {
    if (expected == actual) {
        return;
    }
    fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
    exit(1);
}

void runtest() {
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);
    for (long long int i = 0; i < 100; i++) {
        vec_push(vec, (void *)i);
    }
    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long long)vec->data[0]);
    expect(__LINE__, 50, (long long)vec->data[50]);
    expect(__LINE__, 99, (long long)vec->data[99]);

    printf("OK\n");
}


//エラー用のprintf printfと同じ引数 可変長引数 stdarg.h
void error(char *fmt, ...) {  // ... の部分がapにあたる
    va_list ap;
    va_start(ap, fmt);  //apに値をセット fmtはその情報
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を報告
void error_at(char *loc, char *msg) {
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}
