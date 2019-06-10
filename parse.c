#include "9cc.h"

char *user_input;


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

Node *expr(Vector *tokens) {
    return equality(tokens);
}

Node *equality(Vector *tokens) {
    Node *node = relational(tokens);

    for (;;) {
        if (consumev("==", tokens)) {
            node = new_nodev("==", node, relational(tokens));
        }
        else if (consumev("!=", tokens)) {
            node = new_nodev("!=", node, relational(tokens));
        }
        else {
            return node;
        }
    }
}

Node *relational(Vector *tokens) {
    Node *node = add(tokens);

    for (;;) {
        if (consume('<', tokens)) {
            node = new_node('<', node, add(tokens));
        }
        else if (consumev("<=", tokens)) {
            node = new_nodev("<=", node, add(tokens));
        }
        else if (consume('>', tokens)) {
            node = new_node('<', add(tokens), node);
        }
        else if (consumev(">=", tokens)) {
            node = new_nodev("<=", add(tokens), node);
        }
        else {
            return node;
        }
    }
}

Node *add(Vector *tokens) {
    Node *node = mul(tokens);

    for (;;) {
        if (consume('+', tokens)){
            node = new_node('+', node, mul(tokens));
        }
        else if (consume('-', tokens)){
            node = new_node('-', node, mul(tokens));
        }
        else {
            return node;
        }
    }
}

Node *mul(Vector *tokens) {
    Node *node = unary(tokens);

    for (;;) {
        if (consume('*', tokens)){
            node = new_node('*', node, unary(tokens));
        }
        else if (consume('/', tokens)){
            node = new_node('/', node, unary(tokens));
        }
        else {
            return node;
        }
    }
}

Node *unary(Vector *tokens) {
    if (consume('+', tokens)) {
        return term(tokens);
    }
    if (consume('-', tokens)) {
        return new_node('-', new_node_num(0), term(tokens));
    }
    return term(tokens);
}

Node *term(Vector *tokens) {
    if (consume('(', tokens)) {
        Node *node = expr(tokens);
        if (!consume(')', tokens)){
            error_at(((Token *)tokens->data[pos])->input, "開きカッコに対応する閉じカッコがありません");
        }
        return node;
    }

    if (((Token *)tokens->data[pos])->ty == TK_NUM) {
        return new_node_num(((Token *)tokens->data[pos++])->val);
    }

    error_at(((Token *)tokens->data[pos])->input, "数値でも開きカッコでもないトークンです");
    
    return (Node *)NULL;

}

// user_inputの文字列をトークナイズしてtokensに保存
Vector *tokenize(char *user_input) {
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

int consume(int ty, Vector *tokens) {
    if (((Token *)tokens->data[pos])->ty != ty) {
        return 0;
    }
    pos++;
    return 1;
}

int consumev(char *ty, Vector *tokens) {
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
