#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す列挙型
enum {
    TK_NUM = 256,  // 整数トークン
    TK_EOF,        // 入力の終わりを表すトークン 257を割り当て
};

typedef struct {
    int ty;       // トークンの型
    int val;      // tkがTK_NUMのときの数値
    char *input;  // トークン文字列(エラーメッセージ用)
} Token;

// 入力プログラム
char *user_input;

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

        if (*p == '+' || *p == '-') {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
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

int main(int argc, char **argv){
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    //トークナイズする
    user_input = argv[1];
    tokenize();

    //アセンブリの前半を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 最初は数値のみなので,それをチェック
    if (tokens[0].ty != TK_NUM) {
        error_at(tokens[0].input, "数ではありません");
    }
    printf("    mov rax, %d\n", tokens[0].val);

    // + NUM  か  - NUM というトークンを消費しながら アセンプリ出力
    int i = 1;
    while (tokens[i].ty != TK_EOF) {
        if (tokens[i].ty == '+') {
            i++;
            if (tokens[i].ty != TK_NUM) {
                error_at(tokens[i].input, "数ではありません");
            }
            printf("    add rax, %d\n", tokens[i].val);
            i++;
            continue;
        }


        if (tokens[i].ty == '-') {
            i++;
            if (tokens[i].ty != TK_NUM) {
                error_at(tokens[i].input, "数ではありません");
            }
            printf("    sub rax, %d\n", tokens[i].val);
            i++;
            continue;
        }

        error_at(tokens[i].input, "予期しないトークンです");
    }

    printf("   ret\n");
    return 0;
}
