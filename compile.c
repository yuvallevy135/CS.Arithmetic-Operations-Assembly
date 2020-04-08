//
// Created by yuvallevy on 03/01/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void findTableSize();

int numCase(char *line);

void smallAndBigCase(int num);

void printFirstLines(int arr_size, FILE *fileWrite);

//void printJumpTable(int arr[], int size, FILE *fileWrite);

void printLineAsembly(char *left, char *sign, char *right, FILE *fileWrite);

int stringLen(char *string);

int small = -1;
int big = -1;


int main() {
    // find the size of the jumping table.
    findTableSize();
//    printf("%d\n", big);
//    printf("%d\n", small);
    // this is the size of the JT.
    int arr_size = big - small + 1;
    int jumpTable = big + 3;
    int defaultCase = big + 1;
    int doneCase = big + 2;
    // creating the arr of the JT.
    int arr[arr_size];
    int i;
    // initialize the JT with 0;
    for (i = 0; i < arr_size - 1; i++) {
        arr[i] = -1;
    }

    // printing the first lines on the switch.s
    FILE *fileWrite;
    fileWrite = fopen("switch.s", "w");
    if (fileWrite == NULL) {
        printf("Unable to create file.\n");
    }
    printFirstLines(arr_size, fileWrite);

    //print the asembly now
    char *word = "case";
    int theNumCase = 0;
    int startPrint = 0;
    FILE *fileRead;
    fileRead = fopen("switch.c", "r");
    if (fileRead == NULL) {
        printf("Unable to read file.\n");
    }
    char line[128];
    for (i = 0; i < 4; i++) {
        fgets(line, sizeof(line), fileRead);
//        printf("%s", line);
    }
    while (fgets(line, sizeof(line), fileRead) != NULL) {
        char *foundCase;
        // found a case line.
        if ((foundCase = strstr(line, "case")) != NULL) {
            // print num of the case.
            theNumCase = numCase(line);
            fprintf(fileWrite, ".L%d:\n", theNumCase);
            arr[theNumCase - small] = theNumCase;
        } else if ((foundCase = strstr(line, "break")) != NULL) {
            // break case - go to done.
            fprintf(fileWrite, "jmp .L%d\n", doneCase);

        } else if ((foundCase = strstr(line, "default")) != NULL) {
            // default case.
            fprintf(fileWrite, ".L%d:\n", defaultCase);
        } else if ((foundCase = strstr(line, "}")) != NULL) {
            // ignore "}".
            continue;
        } else if ((foundCase = strstr(line, "return")) != NULL) {
            // return case.
            fprintf(fileWrite, ".L%d:\n", doneCase);
            fprintf(fileWrite, "ret\n");
        } else {
            char *lineArr[3];
            int i = 0;
            char *token = strtok(line, " ");
            // parse the line int 3 pieces.
            while (token != NULL) {
                if (i == 3) {
                    token = strtok(NULL, " ;");
                    continue;
                }
                lineArr[i] = token;
//                printf("%s", lineArr[i]);
                token = strtok(NULL, " ;");
                i++;
            }
            char *left = lineArr[0];
            char *sign = lineArr[1];
            char *right = lineArr[2];
            printLineAsembly(left, sign, right, fileWrite);

        }
    }
    int jTableCase = big + 3;
    fprintf(fileWrite, ".section .rodata\n");
    fprintf(fileWrite, ".align 8\n");
    fprintf(fileWrite, ".L%d:\n", jTableCase);
    int j;
    for (j = 0; j < arr_size; j++) {
        if (arr[j] == -1) {
            fprintf(fileWrite, ".quad .L%d\n", big + 1);
        } else {
            fprintf(fileWrite, ".quad .L%d\n", arr[j]);
        }
    }
    fclose(fileRead);
}


void findTableSize() {
    char *word = "case";
    int theNumCase = 0;
    FILE *fileRead;
    fileRead = fopen("switch.c", "r");
    if (fileRead == NULL) {
        printf("Unable to read file.\n");
    }
    char line[128];
    while (fgets(line, sizeof(line), fileRead) != NULL) {
        char *found = strstr(line, word);
        if (found) {
            theNumCase = numCase(line);
            smallAndBigCase(theNumCase);
        }
    }
    fclose(fileRead);
}

int numCase(char *line) {
    int caseNum = 0, returnNum;
    char *token = strtok(line, " ");
    while (token != NULL) {
        if (caseNum == 0) {
            caseNum++;
            token = strtok(NULL, " ;");
        } else {
            char *theCase = token;
            int len = strlen(theCase);
            theCase[len - 1] = 0;
            len = strlen(theCase);
            theCase[len - 1] = 0;
//            printf("%s\n", token);
            token = strtok(NULL, " ;");
            returnNum = atoi(theCase);
        }
    }
    return returnNum;
}

void smallAndBigCase(int num) {
    if (small == -1 && big == -1) {
        small = num;
        big = num;
    } else {
        if (num < small && num >= 0) {
            small = num;
        }
        if (num > big) {
            big = num;
        }
    }
}

void printFirstLines(int arr_size, FILE *fileWrite) {
    int defaultCase = big + 1, jTableCase = big + 3;
    fprintf(fileWrite, ".section .text\n");
    fprintf(fileWrite, ".globl switch2\n");
    fprintf(fileWrite, "switch2:\n");
    // result = 0;
    fprintf(fileWrite, "movq $0, %%rax\n");
    fprintf(fileWrite, "subq $%d, %%rdx\n", small);
    fprintf(fileWrite, "cmpq $%d, %%rdx\n", arr_size - 1);
    fprintf(fileWrite, "ja .L%d\n", defaultCase);
    fprintf(fileWrite, "jmp *.L%d(,%%rdx,8)\n", jTableCase);
}

//void printJumpTable(int arr[], int size, FILE *fileWrite) {
//    int jTableCase = big + 3;
//    fprintf(fileWrite, ".section .rodata\n");
//    fprintf(fileWrite, ".align 8\n");
//    fprintf(fileWrite, ".L%d 8\n,",jTableCase);
//    int i;
//    for (i = 1; i <= size; i++) {
//        if (arr[i] == -1) {
//            fprintf(fileWrite, ".quad .L%d\n", big + 1);
//        } else {
//            fprintf(fileWrite, ".quad .L%d\n", arr[i]);
//        }
//    }
//}

void printLineAsembly(char *dst, char *sign, char *src, FILE *fileWrite) {
    // %rdi = *p1, %rsi = *p2, %rax = result
    // decide who is dst and who is src.
    char *dstToPrint;
    char *srcToPrint;
    int numFlag = 0;

    if (strcmp(dst, "*p1") == 0) {
        dstToPrint = "(%rdi)";
    } else if (strcmp(dst, "*p2") == 0) {
        dstToPrint = "(%rsi)";
    } else if (strcmp(dst, "result") == 0) {
        dstToPrint = "%rax";
    } else {
        // a number
        int lenOfDst = stringLen(dst);
        char dest2[30];
        memcpy(dest2 + 1, dst, lenOfDst);
        dest2[0] = '$';
        dest2[stringLen(dst) + 1] = '\0';
        dstToPrint = dest2;
        numFlag = 1;
    }
    if (strcmp(src, "*p1") == 0) {
        srcToPrint = "(%rdi)";
    } else if (strcmp(src, "*p2") == 0) {
        srcToPrint = "(%rsi)";
    } else if (strcmp(src, "result") == 0) {
        srcToPrint = "%rax";
    } else {
        // a number
        int lenOfSrc = stringLen(src);
        char src2[30];
        memcpy(src2 + 1, src, lenOfSrc);
        src2[0] = '$';
        src2[stringLen(src) + 1] = '\0';
        srcToPrint = src2;
        numFlag = 1;
    }
    switch (sign[0]) {
        case '=':
            // need to use rcx.
            if (strcmp(dst, "result") != 0 && strcmp(src, "result") != 0 && numFlag == 0) {
                fprintf(fileWrite, "movq %s, %%rcx\n", srcToPrint);
                fprintf(fileWrite, "movq %%rcx, %s\n", dstToPrint);
            } else {
                // can move straight to result.
                fprintf(fileWrite, "movq %s, %s\n", srcToPrint, dstToPrint);
            }
            break;
            // "+="
        case '+':
            // need to use rcx.
            if (strcmp(dst, "result") != 0 && strcmp(src, "result") != 0 && numFlag == 0) {
                fprintf(fileWrite, "movq %s, %%rcx\n", srcToPrint);
                fprintf(fileWrite, "addq %%rcx, %s\n", dstToPrint);
            } else {
                // can move straight to result.
                fprintf(fileWrite, "addq %s, %s\n", srcToPrint, dstToPrint);
            }
            break;
            // "-="
        case '-':
            // need to use rcx.
            if (strcmp(dst, "result") != 0 && strcmp(src, "result") != 0 && numFlag == 0) {
                fprintf(fileWrite, "movq %s, %%rcx\n", srcToPrint);
                fprintf(fileWrite, "subq %%rcx, %s\n", dstToPrint);
            } else {
                // can move straight to result.
                fprintf(fileWrite, "subq %s, %s\n", srcToPrint, dstToPrint);
            }
            break;
            // "*="
        case '*':
            // need to use rcx.
            if (strcmp(dst, "result") != 0 && strcmp(src, "result") != 0 && numFlag == 0) {
                fprintf(fileWrite, "movq %s, %%rcx\n", srcToPrint);
                fprintf(fileWrite, "imulq %%rcx, %s\n", dstToPrint);
            } else {
                // can move straight to result.
                fprintf(fileWrite, "imulq %s, %s\n", srcToPrint, dstToPrint);
            }
            break;
            // ">>="
        case '>':
            if (strcmp(src, "result") != 0 && numFlag == 0) {
                fprintf(fileWrite, "movq %s, %%rcx\n", srcToPrint);
                fprintf(fileWrite, "shr %%cl, %s\n", dstToPrint);
            } else {
                fprintf(fileWrite, "shr %s, %s\n", srcToPrint, dstToPrint);
            }
            break;
            // "<<="
        case '<':
            if (strcmp(src, "result") != 0 && numFlag == 0) {
                fprintf(fileWrite, "movq %s, %%rcx\n", srcToPrint);
                fprintf(fileWrite, "shl %%cl, %s\n", dstToPrint);
            } else {
                fprintf(fileWrite, "shl %s, %s\n", srcToPrint, dstToPrint);
            }
            break;
        default:
            break;
    }
}

int stringLen(char *string) {
    int i;
    for (i = 0; string[i] != '\0'; ++i);
    return i;
}

