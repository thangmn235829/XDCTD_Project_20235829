#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LENGTH 50
#define MAX_INDEX_COUNT 10000000
#define MAX_STOPW_COUNT 10000
#define MAX_LINE_COUNT 100000

typedef struct {
    char word[MAX_WORD_LENGTH];
    int appearLine[MAX_LINE_COUNT];
    int appearLineCount;
} INDEX;

INDEX indexList[MAX_INDEX_COUNT]; int indexCount = 0;
char stopwList[MAX_STOPW_COUNT][MAX_WORD_LENGTH]; int stopwCount = 0;

FILE* openFile(char* fileName) {
    FILE* temp = fopen(fileName, "r+");
    if (temp == NULL) {
        printf("Error opening file %s\n", fileName);
        return NULL;
    }
    else printf("File %s opened successfully\n", fileName);
    return temp;
}

int isDuplicate(char wordList[MAX_STOPW_COUNT][MAX_WORD_LENGTH], int count, const char *word) {
    for (int i = 0; i < count; i++) {
        if (strcmp(wordList[i], word) == 0)
            return 1;
    }
    return 0;
}

void toLowerStr(char *s) {
    while (*s) {
        *s = tolower((unsigned char)*s);
        s++;
    }
}

void getStopWList(FILE* text, char wordList[MAX_STOPW_COUNT][MAX_WORD_LENGTH]) {
    int c, prev = 0, next;
    char buffer[MAX_WORD_LENGTH];
    int idx = 0;
    int wordCount = 0;

    while ((c = fgetc(text)) != EOF) {

        if (isalpha(c)) {
            /* Start accumulating a word */
            buffer[idx++] = c;

            /* Cap to avoid overflow */
            if (idx >= MAX_WORD_LENGTH - 1)
                idx = MAX_WORD_LENGTH - 2;
        } else {
            /* End of an alphabetic block -> complete word candidate */
            if (idx > 0) {
                buffer[idx] = '\0';

                /* Look at prev and next characters to check adjacency to digits */
                next = c;  // current non-alpha acts as "next"
                int invalid = 0;

                if (isdigit(prev) || isdigit(next))
                    invalid = 1;

                if (!invalid) {
                    toLowerStr(buffer);

                    if (!isDuplicate(wordList, wordCount, buffer)) {
                        strcpy(wordList[wordCount++], buffer);
                    }
                }

                idx = 0;  // reset buffer
            }
        }

        prev = c;
    }

    /* Handle last word if file ends with a letter */
    if (idx > 0) {
        buffer[idx] = '\0';
        next = EOF;

        int invalid = 0;
        if (isdigit(prev))
            invalid = 1;

        if (!invalid) {
            toLowerStr(buffer);
            if (!isDuplicate(wordList, wordCount, buffer)) {
                strcpy(wordList[wordCount++], buffer);
            }
        }
    }
}

// Helper: check if word is filtered
int isStopW(const char *word) {
    return isDuplicate(stopwList, stopwCount, word);
}

// Helper: find word in indexList
int findWordIndex(const char *word) {
    for (int i = 0; i < indexCount; i++) {
        if (strcmp(indexList[i].word, word) == 0)
            return i;
    }
    return -1;
}

void getWordList(FILE* text, INDEX indexList[MAX_INDEX_COUNT]) {
    int c, prev = 0, next;
    char buffer[MAX_WORD_LENGTH];
    int idx = 0;
    int line = 1;
    int startOfSentence = 1; // start of file counts as sentence start

    while ((c = fgetc(text)) != EOF) {
        // track newlines
        if (c == '\n') {
            line++;
            startOfSentence = 1; // new line = start of sentence
        }

        if (isalpha(c)) {
            buffer[idx++] = c;
            if (idx >= MAX_WORD_LENGTH - 1)
                idx = MAX_WORD_LENGTH - 2;
        } else {
            if (idx > 0) {
                buffer[idx] = '\0';
                next = c;

                // Check digits on either side
                int invalid = 0;
                if (isdigit(prev) || isdigit(next))
                    invalid = 1;

                // Original uppercase first char
                int originalUpper = isupper(buffer[0]);

                if (!invalid) {
                    if (!(originalUpper && !startOfSentence)) { // filter uppercase not at sentence start
                        char lowerWord[MAX_WORD_LENGTH];
                        strcpy(lowerWord, buffer);
                        toLowerStr(lowerWord);

                        if (!isStopW(lowerWord)) {
                            int pos = findWordIndex(lowerWord);
                            if (pos >= 0) {
                                // Word exists: add line if new
                                int alreadyInLine = 0;
                                for (int i = 0; i < indexList[pos].appearLineCount; i++) {
                                    if (indexList[pos].appearLine[i] == line) {
                                        alreadyInLine = 1;
                                        break;
                                    }
                                }
                                if (!alreadyInLine)
                                    indexList[pos].appearLine[indexList[pos].appearLineCount++] = line;
                            } else {
                                // New word
                                strcpy(indexList[indexCount].word, lowerWord);
                                indexList[indexCount].appearLine[0] = line;
                                indexList[indexCount].appearLineCount = 1;
                                indexCount++;
                            }
                        }
                    }
                }
                idx = 0;
            }

            // After punctuation + space, next is start of sentence
            if ((prev == '.' || prev == '?' || prev == '!') && c == ' ')
                startOfSentence = 1;
            else if (!isspace(c))
                startOfSentence = 0;
        }

        prev = c;
    }

    // Handle last word if file ends with a letter
    if (idx > 0) {
        buffer[idx] = '\0';
        next = EOF;
        int invalid = isdigit(prev);
        int originalUpper = isupper(buffer[0]);
        if (!invalid && !(originalUpper && !startOfSentence)) {
            char lowerWord[MAX_WORD_LENGTH];
            strcpy(lowerWord, buffer);
            toLowerStr(lowerWord);

            if (!isStopW(lowerWord)) {
                int pos = findWordIndex(lowerWord);
                if (pos >= 0) {
                    int alreadyInLine = 0;
                    for (int i = 0; i < indexList[pos].appearLineCount; i++)
                        if (indexList[pos].appearLine[i] == line)
                            alreadyInLine = 1;
                    if (!alreadyInLine)
                        indexList[pos].appearLine[indexList[pos].appearLineCount++] = line;
                } else {
                    strcpy(indexList[indexCount].word, lowerWord);
                    indexList[indexCount].appearLine[0] = line;
                    indexList[indexCount].appearLineCount = 1;
                    indexCount++;
                }
            }
        }
    }
}

// Comparison function for qsort
int compareIndexWords(const void *a, const void *b) {
    const INDEX *ia = (const INDEX *)a;
    const INDEX *ib = (const INDEX *)b;
    return strcmp(ia->word, ib->word);
}

int main()
{
    // open files
    char textName[] = "thuchanh1.txt"; // name of the input text
    char stopwFileName[] = "stopw.txt"; // name of the file containing stopwords
    FILE* textFile = openFile(textName);
    FILE* stopwFile = openFile(stopwFileName);
    if ((textFile == NULL) != (stopwFile == NULL)) return 1;

    // read stopwords
    getStopWList(stopwFile, stopwList);
    printf("===============>STOP WORDS<===============\n");
    for (stopwCount = 0; stopwList[stopwCount][0] != '\0'; stopwCount++)
        printf("%s\n", stopwList[stopwCount]);

    // read words
    getWordList(textFile, indexList);

    // Sort indexList alphabetically by word
    qsort(indexList, indexCount, sizeof(INDEX), compareIndexWords);
    printf("===============>WORDS<===============\n");
    for (int i = 0; i < indexCount; i++) {
        printf("%s %d", indexList[i].word, indexList[i].appearLineCount);
        for (int j = 0; indexList[i].appearLine[j] > 0; j++)
            printf(", %d", indexList[i].appearLine[j]);
        printf("\n");
    }

    // close files
    fclose(textFile);
    fclose(stopwFile);
    return 0;
}
