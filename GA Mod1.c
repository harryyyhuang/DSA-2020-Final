#include "api.h"
#include <stdio.h>

typedef struct person{
    char name[32];
    int rank, size;
    struct person *next;
    struct person *parent;
}Person;

int len, ret, j, count = 0, largest = 0;
int n_mails, n_queries;
mail *mails;
query *queries;

Person* find(Person* current){
    if(current->parent != current){
        //Path compression
        current->parent = find(current->parent);
        return current->parent;
    }
    return current;
}

int compareMax(int a, int b){
    if(a >= b)
        return a;
    else
        return b;
}

int c2i(char c) {
    if ('a' <= c && c <= 'z')
        return c - 'a';
    else if ('A' <= c && c <= 'Z')
        return c - 'A' + 26;
    else
        return -1;
}

int getHashValue(char str[], int all){
    ret = 0;
    len = (int)strlen(str);
    for(j = 0; j < len; j++){
        ret = (52 * ret + c2i(str[j])) % all;
    }
    return ret;
}

void collision(Person* y, char xName[]){
    Person* new = (Person*)malloc(sizeof(Person));
    int xNameLen = (int)strlen(xName);
    for(j = 0; j< xNameLen; j++){
        new->name[j] = xName[j];
    }
    y->next = new;
    new->next = new;
    new->parent = new;
    new->rank = 0;
    new->size = 0;
}

Person* checkExist(Person* y, char xName[]){
    int xNameLen = (int)strlen(xName);
    if(y->name[0] == '\0'){
        for(j = 0; j< xNameLen; j++){
            y->name[j] = xName[j];
        }
        return y;
    }else{
        if(xNameLen == strlen(y->name)){
            for(j = 0; j< xNameLen; j++){
                if(xName[j] != y->name[j])
                    break;
            }
            if(j == xNameLen)
                return y;
        }
        if(y->next != y)
            return checkExist(y->next, xName);
        else{
            collision(y, xName);
            return y->next;
        }
    }
}

void unionByRank(Person* x, Person* y){
    Person* xRoot = find(x);
    Person* yRoot = find(y);
    Person *large, *small;

    if(xRoot != yRoot){
        if(xRoot->rank < yRoot->rank){
            large = yRoot;
            small = xRoot;
        }else{
            large = xRoot;
            small = yRoot;
        }
        small->parent = large;
        large->size += small->size;
        count -= 1;
        compareMax(largest, large->size);
        if(large->rank == small->rank)
            large->rank += 1;
    }
}


int* group_analyses(int mids[]){
    int all = sizeof(*mids)/sizeof(int);
    Person *x, *y;
    Person people = (Person*)malloc(sizeof(Person)* all);
    int value;
    //Init
    for(int i = 0; i< all; i++){
        people[i]->name[0] = '\0';
        people[i]->next = people[i];
        people[i]->parent = people[i];
        people[i]->rank = 0;
        people[i]->size = 0;
    }
    count = all;
    for(int i = 0; i< all; i++){
        value = getHashValue(mails[mids[i]].from, all);
        x = checkExist(people[value], mails[mids[i]].from);
        value = getHashValue(mails[mids[i]].to, all);
        y = checkExist(people[value], mails[mids[i]].to);
        unionByRank(x, y);
    }
    int ans[2] = {count, largest};
    free(people);
    return ans;
}

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);
    printf("Initialization finished\n");
    for(int i = 0; i < n_queries; i++)
        if(queries[i].type == expression_match)
          api.answer(queries[i].id, NULL, 0);
        else if(queries[i].type == group_analyse){
            group_analyses(mids);
        }

  return 0;
}

