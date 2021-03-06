#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define NUMBERS 100
#define REGULAR 3000
#define SPLITTING 20000
#define ADDING 2000

typedef struct Number
{
    int value;
    struct Number *left;
    struct Number *right;
    struct Number *parent;
} Number;

int part1(FILE *in);
int part2(FILE *in);
Number *readNumber(FILE *in, Number *buff);
Number *read(FILE *in, Number *buff, Number *array);
Number add(Number *a, Number *b);
int reduce(Number *num, int depth, Number *buff, int *i, int state);
void explode(Number *num);
void split(Number *num, Number *buff);
int magnitude(Number *num);
void print(Number *num, int depth);
// A bit sad that I need this, but I don't want to debug
void ensureParent(Number *root);

int main()
{
    FILE *in = fopen("in18", "r");

    printf("Part1: %d\n", part1(in));
    rewind(in);
    printf("Part2: %d\n", part2(in));

    fclose(in);
    return 0;
}

int part1(FILE *in)
{
    Number buff[NUMBERS] = {0};
    Number regularBuff[REGULAR] = {0};
    Number reduceBuff[SPLITTING] = {0};
    Number addBuff[ADDING] = {0};
    int reduceI = 0;

    Number *numbers = read(in, regularBuff, buff);

    Number curr = *numbers;
    while (reduce(&curr, 0, reduceBuff, &reduceI, 0) ||
            reduce(&curr, 0, reduceBuff, &reduceI, 1))
        ;
    for (int i = 1; i < NUMBERS; i++)
    {
        addBuff[i] = curr;
        addBuff[i].left->parent = &addBuff[i];
        addBuff[i].right->parent = &addBuff[i];
        curr = add(&addBuff[i], numbers+i);
        curr.left->parent = &curr;
        curr.right->parent = &curr;
        while (reduce(&curr, 0, reduceBuff, &reduceI, 0) ||
                reduce(&curr, 0, reduceBuff, &reduceI, 1))
            ;
    }

    return magnitude(&curr);
}

int part2(FILE *in)
{
    int maxMagnitude = 0;

    for (int i = 0; i < NUMBERS; i++)
    {
        for (int j = 0; j < NUMBERS; j++)
        {
            Number buff[NUMBERS] = {0};
            Number regularBuff[REGULAR] = {0};
            Number reduceBuff[SPLITTING] = {0};
            int reduceI = 0;
            Number *numbers = read(in, regularBuff, buff);
            rewind(in);

            Number curr = add(numbers+i, numbers+j);
            curr.left->parent = &curr;
            curr.right->parent = &curr;
            while (reduce(&curr, 0, reduceBuff, &reduceI, 0) ||
                    reduce(&curr, 0, reduceBuff, &reduceI, 1))
                ;
            int mag = magnitude(&curr);
            if (mag > maxMagnitude)
                maxMagnitude = mag;
        }
    }

    return maxMagnitude;
}

Number *readNumber(FILE *in, Number *buff)
{
    int c = fgetc(in);
    if (c >= '0' && c <= '9')
    {
        buff->value = c - '0';
        return buff;
    }
    if (c != '[')
    {
        fprintf(stderr, "Expected '[', got '%c'\n", c);
        exit(1);
    }
    Number *ptr = readNumber(in, buff+1);
    buff->left = buff+1;
    buff->left->parent = buff;
    c = fgetc(in);
    if (c != ',')
    {
        fprintf(stderr, "Expected ',', got '%c'\n", c);
        exit(2);
    }
    Number *ptr2 = readNumber(in, ptr+1);
    buff->right = ptr+1;
    buff->right->parent = buff;
    c = fgetc(in);
    if (c != ']')
    {
        fprintf(stderr, "Expected ']', got '%c'\n", c);
        exit(3);
    }
    return ptr2;
}

Number *read(FILE *in, Number *buff, Number *array)
{
    Number *current = buff;;
    for (int i = 0; i < NUMBERS; i++)
    {
        Number *end = readNumber(in, current);
        array[i] = *current;
        current = end+1;
        int c = fgetc(in);
        if (c != '\n')
        {
            fprintf(stderr, "Expected line break, got '%c'\n", c);
            exit(4);
        }
    }
    return array;
}

Number add(Number *a, Number *b)
{
    Number res = {0};
    res.left = a;
    res.right = b;
    return res;
}

int reduce(Number *num, int depth, Number *buff, int *i, int state)
{
    if (num == NULL)
        return 0;
    ensureParent(num);
    if (state == 0 && depth >= 4 && num->left != NULL)
    {
        explode(num);
        return 1;
    }
    if (state == 1 && num->left == NULL && num->value >= 10)
    {
        split(num, buff+*i);
        *i += 2;
        return 2;
    }
    int res = reduce(num->left, depth+1, buff, i, state) ||
        reduce(num->right, depth+1, buff, i, state);
    return res;
}

void explode(Number *num)
{
    // Get regular number to left
    Number *current = num;
    Number *parent = NULL;
    while ((parent = current->parent) && parent->left == current)
        current = parent;
    // Now current is the right child of parent
    if (parent != NULL)
        parent = parent->left;
    while (parent != NULL && (current = parent->right))
        parent = current;
    // Now parent is the right-most value to the left of num
    if (parent != NULL)
        parent->value += num->left->value;

    // Get regular number to right
    current = num;
    parent = NULL;
    while ((parent = current->parent) && parent->right == current)
        current = parent;
    // Now current is the left child of parent
    if (parent != NULL)
        parent = parent->right;
    while (parent != NULL && (current = parent->left))
        parent = current;
    // Now parent is the left-most value to the right of num
    if (parent != NULL)
        parent->value += num->right->value;

    // Remove exploded pair
    num->value = 0;
    num->left = NULL;
    num->right = NULL;
}

void split(Number *num, Number *buff)
{
    num->left = buff;
    num->right = buff+1;
    num->left->parent = num;
    num->right->parent = num;
    num->left->value = num->value / 2;
    num->right->value = num->value - num->left->value;
}

int magnitude(Number *num)
{
    if (num->left == NULL)
        return num->value;
    return 3 * magnitude(num->left) + 2 * magnitude(num->right);
}

void print(Number *num, int depth)
{
    if (num == NULL)
    {
        printf("NULL");
        return;
    }
    if (depth <= 0)
    {
        printf("...");
        return;
    }
    if (num->left == NULL)
        printf("%d", num->value);
    else
    {
        printf("[");
        print(num->left, depth-1);
        printf(",");
        print(num->right, depth-1);
        printf("]");
    }
}

void ensureParent(Number *root)
{
    if (root->left != NULL)
    {
        root->left->parent = root;
        ensureParent(root->left);
    }
    if (root->right != NULL)
    {
        root->right->parent = root;
        ensureParent(root->right);
    }
}

