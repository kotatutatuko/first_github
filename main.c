#include "9cc.h"

int pos = 0;  // この様に宣言だけでなくどこかのファイルで定義しなければならない
              // 初期値は設定しなくてもよい！！！ あとどこのファイルでも良い

int main(int argc, char **argv){
    if (argc == 2 && !strcmp(argv[1], "-test")) {
        runtest();
        return 0;
    }
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    user_input = argv[1];

    Vector *tokens = tokenize(user_input);
    Node *node = expr(tokens);

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
