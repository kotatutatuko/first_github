#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す列挙型
enum {
    TK_NUM = 256,  // 整数トークン
    TK_EOF,       // 入力の終わりを表すトークン 257を割り当て
    TK_EQ,
    TK_NE,
    TK_LE,
    TK_GE
};

typedef struct {
    int ty;       // トークンの型
    int val;      // tkがTK_NUMのときの数値
    char *input;  // トークン文字列(エラーメッセージ用)
} Token;

typedef struct Node {
    int ty;            // 型
    struct Node *lhs;  // 左辺 left hand side
    struct Node *rhs;
    int val;           // 数値の場合使う
} Node;

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * 16); // void *  はvoidのポインタサイズ
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

Token *add_token(Vector *vec, int ty, char *input) {
    Token *t = malloc(sizeof(Token));
    t->ty = ty;
    t->input = input;
    vec_push(vec, t);
    return t;
}

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();


// 入力プログラム
char *user_input;

// トークンのインデックス
int pos;

// トークナイズしたものを入れる配列 100個限定
Vector *tokens;
//tokens = malloc(sizeof(Vector))

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

// user_inputの文字列をトークナイズしてtokensに保存
Vector *tokenize() {
    char *p = user_input;
    Vector *vec = new_vector();

    int i = 0;
    while (*p) {
        // 空白をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (strncmp(p, "==", 2) == 0) {
            add_token(vec, TK_EQ, "==");
            p += 2;
            i++;
            continue;
        }

        if (strncmp(p, "!=", 2) == 0) {
            add_token(vec, TK_NE, "!=");
            p += 2;
            i++;
            continue;
        }

        if (strncmp(p, "<=", 2) == 0) {
            add_token(vec, TK_LE, "<=");
            p += 2;
            i++;
            continue;
        }

        if (strncmp(p, ">=", 2) == 0) {
            add_token(vec, TK_GE, ">=");
            p += 2;
            i++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>') {
            add_token(vec, *p, p);
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            Token *t = add_token(vec, TK_NUM, p);
            t->val = strtol(p, &p, 10);
            i++;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    add_token(vec, TK_EOF, p);
    return vec;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_nodev(char *ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    if (strncmp(ty, "==", 2) == 0) {
        node->ty = TK_EQ;
    }
    else if (strncmp(ty, "!=", 2) == 0) {
        node->ty = TK_NE;
    }
    else if (strncmp(ty, "<=", 2) == 0) {
        node->ty = TK_LE;
    }
    else if (strncmp(ty, ">=", 2) == 0) {
        node->ty = TK_GE;
    }
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = TK_NUM;
    node->val = val;
    return node;
}

int consume(int ty) {
    if (((Token *)tokens->data[pos])->ty != ty) {
        return 0;
    }
    pos++;
    return 1;
}

int consumev(char *ty) {
    int tyr;
    if (strncmp(ty, "==", 2) == 0) {
        tyr = TK_EQ;
    }
    else if (strncmp(ty, "!=", 2) == 0) {
        tyr = TK_NE;
    }
    else if (strncmp(ty, "<=", 2) == 0) {
        tyr = TK_LE;
    }
    else if (strncmp(ty, ">=", 2) == 0) {
        tyr = TK_GE;
    }
    else {
        return 0;
    }
    if (((Token *)tokens->data[pos])->ty != tyr) {
        return 0;
    }
    pos++;
    return 1;

}


void gen(Node *node) {
    if (node->ty == TK_NUM) {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->ty) {
    case '+':
        printf("    add rax, rdi\n");
        break;
    case '-':
        printf("    sub rax, rdi\n");
        break;
    case '*':
        printf("    imul rdi\n");
        break;
    case '/':
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case '<':
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case TK_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case TK_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case TK_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
    }

    printf("    push rax\n");
}

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

int main(int argc, char **argv){
    if (argc == 2 && !strcmp(argv[1], "-test")) {
        runtest();
        return 0;
    }
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    //トークナイズしてパースする
    user_input = argv[1];
    pos = 0;
    tokens = tokenize();
    Node *node = expr();

    //アセンブリの前半を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    //抽象構文木を下りながらコード生成
    gen(node);

    //スタックトップの値をraxにロード
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}

Node *expr() {
    return equality();
}

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consumev("==")) {
            node = new_nodev("==", node, relational());
        }
        else if (consumev("!=")) {
            node = new_nodev("!=", node, relational());
        }
        else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume('<')) {
            node = new_node('<', node, add());
        }
        else if (consumev("<=")) {
            node = new_nodev("<=", node, add());
        }
        else if (consume('>')) {
            node = new_node('<', add(), node);
        }
        else if (consumev(">=")) {
            node = new_nodev("<=", add(), node);
        }
        else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume('+')){
            node = new_node('+', node, mul());
        }
        else if (consume('-')){
            node = new_node('-', node, mul());
        }
        else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*')){
            node = new_node('*', node, unary());
        }
        else if (consume('/')){
            node = new_node('/', node, unary());
        }
        else {
            return node;
        }
    }
}

Node *unary() {
    if (consume('+')) {
        return term();
    }
    if (consume('-')) {
        return new_node('-', new_node_num(0), term());
    }
    return term();
}

Node *term() {
    if (consume('(')) {
        Node *node = expr();
        if (!consume(')')){
            error_at(((Token *)tokens->data[pos])->input, "開きカッコに対応する閉じカッコがありません");
        }
        return node;
    }

    if (((Token *)tokens->data[pos])->ty == TK_NUM) {
        return new_node_num(((Token *)tokens->data[pos++])->val);
    }

    error_at(((Token *)tokens->data[pos])->input, "数値でも開きカッコでもないトークンです");
}
