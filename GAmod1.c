#include "api.h"
#include <stdio.h>

typedef struct person{
    char name[32];
    int rank, size, q_time, nameLen, parent, hashValue;
}Person;

int len, ret, count = 0, largest = 0, collision = 0, outcome = 0, j = 0, this_time, all;
int n_mails, n_queries;
mail *mails;
query *queries;
Person people[30000];

//好像哪裡怪怪的
int find(int current, int this_time){
    int first = current;
    Person cur = people[current];
    while(cur.parent != -1 && cur.q_time == this_time){
        current = cur.parent;
        cur = people[current];
    }
    //如果不是這次弄過的，初始化
    if(cur.q_time != this_time){
        people[current].size = 1;
        people[current].rank = 0;
        people[current].parent = -1;
        people[current].q_time = this_time;
    }
    people[first].parent = current;
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

int getHashValue(char str[]){
    ret = 0;
    int k;
    for(k = 0; k < 32; k++){
        if(str[k] == '\0')
            break;
        else
            ret = (52 * ret + c2i(str[k])) % all;
    }
    len = k;
    return ret;
}

int doubleHash(int hash){
    return (hash + collision * (3 + (hash + 1) % 29)) % 30000;
}

//失敗回傳0，成功回傳1，遇到空洞回傳-1
int isNameMatch(char xName[], int y){
    if(people[y].name[0] == '\0')
        return -1;
    else{
        if(len == people[y].nameLen){
            for(j = 0; j< len; j++)
              if(xName[j] != people[y].name[j])
                   break;
            if(j == len)
                return 1;
            else
                return 0;
        }else
            return 0;
    }
}

int newComing(char name[], int x, int hashValue){
    people[x].nameLen = len;
    for(j = 0; j< 32; j++){
        if(name[j] == '\0')
            break;
        else
            people[x].name[j] = name[j];
    }
    people[x].nameLen = j;
    people[x].rank = 0;
    people[x].size = 1;
    people[x].q_time = this_time;
    people[x].parent = -1;
    people[x].hashValue = hashValue;
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
            return newComing(name, index, hash);
    }else
        return newComing(name, index, hash);

}

void peopleINIT(void){
    for(int i = 0; i< 30000; i++){
        people[i].name[0] = '\0';
        people[i].rank = 0;
        people[i].size = 1;
        people[i].q_time = -1;
        people[i].nameLen = 0;
        people[i].parent = -1;
        people[i].hashValue = 0;
    }
}
//把large跟small改成ptr
void unionByRank(int x, int y){
    people[x].q_time = this_time;
    people[y].q_time = this_time;
    int xRoot = find(x, this_time);
    int yRoot = find(y, this_time);
    people[xRoot].q_time = this_time;
    people[yRoot].q_time = this_time;

    if(xRoot != yRoot){
        if(people[xRoot].rank < people[yRoot].rank){
            people[xRoot].parent = yRoot;
            people[yRoot].size += people[xRoot].size;
            largest = compareMax(largest, people[yRoot].size);
            if(people[yRoot].rank == people[xRoot].rank)
                people[yRoot].rank += 1;
        }else{
            people[yRoot].parent = xRoot;
            people[xRoot].size += people[yRoot].size;
            largest = compareMax(largest, people[xRoot].size);
            if(people[xRoot].rank == people[yRoot].rank)
                people[xRoot].rank += 1;
        }
        count -= 1;
    }
}

void group_analyses(int i){
    this_time = i;
    all = queries[i].data.group_analyse_data.len;
    int mids[all];
    count = all, largest = 0;
    for(int k = 0; k< all; k++)
        mids[k] = queries[i].data.group_analyse_data.mids[k];
    //找關係
    int x, y;
    for(int k = 0; k< all; k++){
        x = checkExist(mails[mids[k]].from);
        y = checkExist(mails[mids[k]].to);
        unionByRank(x, y);
    }
    int ans[2] = {count, largest};
    api.answer(queries[i].id, ans, 2);
}

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);
    peopleINIT();
    for(int i = 0; i < n_queries; i++){
        if(queries[i].type == expression_match)
            api.answer(queries[i].id, NULL, 0);
        else if(queries[i].type == find_similar)
            api.answer(queries[i].id, NULL, 0);
        else if(queries[i].type == group_analyse){
            group_analyses(i);
        }
    }


  return 0;
}
