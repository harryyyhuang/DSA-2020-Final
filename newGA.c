#include "api.h"
#include <stdio.h>

typedef struct person
{
    char name[32];
    int rank, size, q_time, nameLen;
    struct person* parent; 
}Person;

Person **people ;
int **mail_name ; 
int len, ret, count = 0, largest = 0, collision = 0, outcome = 0, j = 0, this_time, all;
int n_mails, n_queries;
mail *mails;
query *queries;


Person* find(Person* current){
    if (current->parent != current && current->q_time == this_time)
        current->parent = find(current->parent);
    return current->parent;
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

int getHashValue(char str[]){
    ret = 0;
    int k;
    for(k = 0; k < 32; k++){
        if(str[k] == '\0')
            break;
        else
            ret = (52 * ret + c2i(str[k])) % 40000;
    }
    len = k;
    return ret;
}

int doubleHash(int hash){
    return (hash + collision + collision*collision) %40000;
}

int isNameMatch(char xName[], int y){

    if(people[y] == NULL)
        return -1;
    else{
        if(len == people[y]->nameLen){
            if(!strcmp(people[y]->name, xName))
                return 1;
            else
                return 0;
        }else
            return 0;
    }
}

int newComing(char name[], int x, int hashValue){
    people[x] = (Person*)malloc(sizeof(Person));
    people[x]->nameLen = len;
    for(j = 0; j< 32; j++){
        if(name[j] == '\0')
            break;
        else
            people[x]->name[j] = name[j];
    }
    people[x]->nameLen = j;
    people[x]->rank = 0;
    people[x]->size = 1;
    people[x]->q_time = -1; //this_time
    people[x]->parent = people[x];
    return hashValue;
}

int checkExist(char name[]){
    collision = 0;
    int hash = getHashValue(name);
    int index = hash;
    outcome = isNameMatch(name, index);
    if(outcome == 1)
        return index;
    else if(outcome == 0){
        while(outcome == 0){
            collision += 1;
            index = doubleHash(hash);
            outcome = isNameMatch(name, index);
        }
        if(outcome == 1)
            return index;
        else
            return newComing(name, index, index);
    }else
        return newComing(name, index, index);

}


void peopleINIT(void){
    people = (Person**)calloc(40000, sizeof(Person*));
    mail_name = (int**)malloc(n_mails*sizeof(int*));
        
    
    int x;
    int y;
    for (size_t i = 0; i < n_mails; i++)
    {
        mail_name[i] = (int*)malloc(2*sizeof(int));
        x = checkExist(mails[i].from);
        y = checkExist(mails[i].to);
        mail_name[i][0] = x;
        mail_name[i][1] = y; 
    }
    
}

void checkQTime(int x, int y){
    if(people[x]->q_time != this_time){
        people[x]->rank = 0;
        people[x]->size = 1;
        people[x]->q_time = this_time;
        people[x]->parent = people[x];
        count+=1;
    }
    if(people[y]->q_time != this_time){
        people[y]->rank = 0;
        people[y]->size = 1;
        people[y]->q_time = this_time;
        people[y]->parent = people[y];
        count+=1;
    } 
}

void unionByRank(int x, int y){
    Person* xRoot = find(people[x]);
    Person* yRoot = find(people[y]);

    if(xRoot != yRoot){
        if(xRoot->rank < yRoot->rank){
            xRoot->parent = yRoot;
            yRoot->size += xRoot->size;
            largest = compareMax(largest, yRoot->size);
            if(yRoot->rank == xRoot->rank)
                yRoot->rank+=1;
        }else{
            yRoot->parent = xRoot;
            xRoot->size += yRoot->size;
            largest = compareMax(largest, xRoot->size);
            if(yRoot->rank == xRoot->rank)
                xRoot->rank+=1;
        }
        count -= 1;
    }
}

void group_analyses(int i){
    this_time = i;
    all = queries[i].data.group_analyse_data.len;
    count = 0, largest = 1;
    //找關係
    int x, y;
    for(int k = 0; k< all; k++){
        x = mail_name[queries[i].data.group_analyse_data.mids[k]][0];
        y = mail_name[queries[i].data.group_analyse_data.mids[k]][1];
        if(x == y)
            continue;
        checkQTime(x, y);
        unionByRank(x, y);

    }
    int ans[2] = {count, largest};
    api.answer(queries[i].id, ans, 2);


}


int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);
    peopleINIT();
    for(int i = 0; i < n_queries; i++){
        // if(queries[i].type == expression_match)
        //     api.answer(queries[i].id, NULL, 0);
        // else if(queries[i].type == find_similar)
        //     api.answer(queries[i].id, NULL, 0);
        if(queries[i].type == group_analyse){
            group_analyses(i);
        }
    }


    return 0;
}