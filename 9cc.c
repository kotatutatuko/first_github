#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す列挙型
enum {
    ND_NUM = 256,  // 整数トークン
    TK_EOF,        // 入力の終わりを表すトークン 257を割り当て
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

Node *expr();
Node *mul();
Node *term();
Node *unary();

// 入力プログラム
char *user_input;

// トークンのインデックス
int pos;

// トークナイズしたものを入れる配列 100個限定
Token tokens[100];

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
void tokenize() {
    char *p = user_input;


    int i = 0;
    while (*p) {
        // 空白をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = ND_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

int consume(int ty) {
    if (tokens[pos].ty != ty) {
        return 0;
    }
    pos++;
    return 1;
}


void gen(Node *node) {
    if (node->ty == ND_NUM) {
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
    }

    printf("    push rax\n");
}

int main(int argc, char **argv){
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    //トークナイズしてパースする
    user_input = argv[1];
    pos = 0;
    tokenize();
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


Node *term() {
    if (consume('(')) {
        Node *node = expr();
        if (!consume(')')){
            error_at(tokens[pos].input, "開きカッコに対応する閉じカッコがありません");
        }
        return node;
    }

    if (tokens[pos].ty == ND_NUM) {
        return new_node_num(tokens[pos++].val);
    }

    error_at(tokens[pos].input, "数値でも開きカッコでもないトークンです");
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

Node *expr() {
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

Node *unary() {
    if (consume('+')) {
        return term();
    }
    if (consume('-')) {
        return new_node('-', new_node_num(0), term());
    }
    return term();
}
