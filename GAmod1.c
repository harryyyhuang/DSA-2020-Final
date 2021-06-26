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
Person people[3000000];
Person defaultNull;

Person find(Person current, int this_time){
    Person cur = current;
    while(cur.parent != -1 && cur.q_time == this_time){
        current.parent = cur.parent;
        cur = people[cur.parent];
        //printf("Update parent\n");
    }
    return cur;
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
    len = (int)strlen(str);
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
    return (hash + collision * (hash % 1121)) % 3000000;
}

//失敗回傳0，成功回傳1，遇到空洞回傳-1
int isNameMatch(char xName[], Person y){
    if(y.name[0] == '\0')
        return -1;
    else{
        if(len == y.nameLen){
            for(j = 0; j< len; j++)
              if(xName[j] != y.name[j])
                   break;
            if(j == len)
                return 1;
            else
                return 0;
        }else
            return 0;
    }
}

Person newComing(char name[], Person x, int hashValue){
    x.nameLen = len;
    for(int k = 0; k< x.nameLen; k++){
        x.name[k] = name[k];
    }
    x.rank = 0;
    x.size = 1;
    x.q_time = this_time;
    x.parent = -1;
    x.hashValue = hashValue;
    return x;
}

Person checkExist(char name[]){
    collision = 0;
    int hash = getHashValue(name);
    int index = hash;
    outcome = isNameMatch(name, people[index]);
    if(outcome == 1)
        return people[index];
    else if(outcome == 0){
        //處理collision時的double hashing.
        while(outcome == 0){
            collision += 1;
            index = doubleHash(hash);
            outcome = isNameMatch(name, people[index]);
        }
        if(outcome == 1)
            return people[index];
        else
            return newComing(name, people[index], hash);
    }else
        return newComing(name, people[index], hash);

}

void peopleINIT(void){
    for(int i = 0; i< 20000; i++){
        people[i].name[0] = '\0';
        people[i].rank = 0;
        people[i].size = 1;
        people[i].q_time = -1;
        people[i].nameLen = 0;
        people[i].parent = -1;
        people[i].hashValue = 0;
    }
}

void unionByRank(Person x, Person y){
    x.q_time = this_time;
    y.q_time = this_time;
    Person xRoot = find(x, this_time);
    xRoot.q_time = this_time;
    Person yRoot = find(y, this_time);
    yRoot.q_time = this_time;
    Person large, small;

    if(&xRoot != &yRoot){
        if(xRoot.rank < yRoot.rank){
            large = yRoot;
            small = xRoot;
        }else{
            large = xRoot;
            small = yRoot;
        }
        small.parent = large.hashValue;
        large.size += small.size;
        count -= 1;
        largest = compareMax(largest, large.size);
        if(large.rank == small.rank)
            large.rank += 1;
    }
}

void group_analyses(int i){
    this_time = i;
    all = queries[i].data.group_analyse_data.len;
    int mids[all], count = all, largest = 0;
    for(int k = 0; k< all; k++)
        mids[k] = queries[i].data.group_analyse_data.mids[k];
    //找關係
    Person x, y;
    for(int k = 0; k< all; k++){
        x = checkExist(mails[mids[k]].from);
        y = checkExist(mails[mids[k]].to);
        //printf("self:    %s %s\n", x.name, y.name);
        //printf("parents: %s %s\n", people[x.parent].name, people[y.parent].name);
        unionByRank(x, y);
    }
    int ans[2] = {count, largest};
    printf("%d %d\n", ans[0], ans[1]);
    api.answer(queries[i].id, ans, 2);
}

int main(void) {
    api.init(&n_mails, &n_queries, &mails, &queries);
    peopleINIT();
    for(int i = 0; i < n_queries; i++){
        if(queries[i].type == expression_match)
            printf("");
            //api.answer(queries[i].id, NULL, 0);
        //else if(queries[i].type == find_similar)
            //api.answer(queries[i].id, NULL, 0);
        else if(queries[i].type == group_analyse){
            group_analyses(i);
        }
    }


  return 0;
}
