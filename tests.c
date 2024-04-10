#ifndef TESTS_C
#define TESTS_C

char green[] = "\033[32m";
char red[] = "\033[31m";
char blue[] = "\033[34m";
char reset[] = "\033[0m";

bool test_tokenizer(){
    Token *tokens = (Token*) malloc(sizeof(Token) * 20);
    if (tokens == NULL) exit(69);

    char src[] = "5 + 6 == 11";

    tokenize(tokens, src);

    printf("%s\n", src);
    printTokens(tokens);

    Token *goal = (Token*) calloc(5, sizeof(Token));
    if (goal == NULL) exit(69);

    TokenData data;
    data.num = 5;
    goal[0].type = TK_NUM; 
    goal[0].data = data;
    goal[1].type = TK_PLUS; 
    data.num = 6;
    goal[2].type = TK_NUM;
    goal[2].data = data;
    goal[3].type = TK_EQ; 
    data.num = 11;
    goal[4].type = TK_NUM; 
    goal[4].data = data;
    goal[5].type = TK_EOF; 

    for (int i =0; i<5; i++) {
        if (compare_tokens(tokens[i], goal[i]) == false) {
            printf("%s", red);
            printf("!! Failed to match tokens !!\n");
            printf("Got:\n");
            printTokens(tokens);
            printf("Expected:\n");
            printTokens(goal);
            printf("%s", reset);
            return false;
        }
    }

    printf("%s", green);
    printf("~~ Succefully Matched tokens ~~\n");
    printf("%s", reset);
    return true;
}

bool test(){
    printf("%sTesting tokenizer%s\n", blue, reset);
    return test_tokenizer();
}

#endif
