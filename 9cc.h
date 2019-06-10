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

void gen(Node *node);
void runtest();
void vec_push(Vector *vec, void *elem);
void error(char *fmt, ...);
void error_at(char *loc, char *msg);

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *term();
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_nodev(char *ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Token *add_token(Vector *vec, int ty, char *input);

Vector *new_vector();
Vector *tokenize();

int consume(int ty, Vector *tokens);
int consumev(char *ty, Vector *tokens);


extern char *user_input;
// トークンのインデックス
extern int pos;
// トークナイズしたものを入れるVector
//extern Vector *tokens;
