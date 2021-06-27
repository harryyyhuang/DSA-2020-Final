#include "api.h"
#include <stdio.h>

typedef struct person{
    char name[32];
    int rank, size, q_time, nameLen, parent, hashValue;
}Person;

int len, ret, count = 0, largest = 0, collision = 0, outcome = 0, j = 0, this_time, all;
int testCount = 0;
int n_mails, n_queries;
mail *mails;
query *queries;
Person people[30000];

//好像哪裡怪怪的
// int find(int current, int this_time){
//     int start = current;
//     Person carrier = people[current], prev;
//     while(carrier.parent != -1 && carrier.parent != carrier.hashValue && carrier.q_time == this_time){
//         prev = carrier;
//         current = carrier.parent;
//         carrier = people[current];
//     }
//     if(carrier.q_time == this_time){
//         people[start].parent = carrier.hashValue;
//     }else{
//         people[start].parent = prev.hashValue;
//     }
//     return people[start].parent;
// }

// pass compression
int find(Person* current){
    if (current->parent != current->hashValue && current->q_time == this_time){
        current->parent = find(&people[current->parent]);
    }
    // if (current->parent != current->hashValue)
    //     current->parent = current->hashValue;
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
    people[x].q_time = -1; //this_time
    people[x].parent = hashValue;
    people[x].hashValue = hashValue;
    count+=1;
    //if(this_time == 3)
    //    printf("New Coming %s\n", people[x].name);
    return hashValue;
}

int checkExist(char name[]){
    collision = 0;
    int hash = getHashValue(name);
    int index = hash;
    outcome = isNameMatch(name, index);
    if(outcome == 1){
        if(people[index].q_time != this_time){
            count+=1;
            people[index].rank = 0;
            people[index].size = 1;
            people[index].parent = index;
        }
        //if(this_time == 3)
        //    printf("Match %s\n", people[index].name);
        return index;
    }
    else if(outcome == 0){
        while(outcome == 0){
            collision += 1;
            index = doubleHash(hash);
            outcome = isNameMatch(name, index);
        }
        if(outcome == 1){
            if(people[index].q_time != this_time){
                count+=1;
                people[index].rank = 0;
                people[index].size = 1;
                people[index].parent = index;
            }
            //if(this_time == 3)
            //    printf("Match %s\n", people[index].name);
            return index;
        }
        else
            return newComing(name, index, index);
    }else
        return newComing(name, index, index);

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
    int xRoot = find(&people[x]);
    int yRoot = find(&people[y]);
    // people[xRoot].q_time = this_time;
    // people[yRoot].q_time = this_time;
    if(xRoot != yRoot){
        // printf("else\n");
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
    count = 0, largest = 1;
    //找關係
    int x, y;
    for(int k = 0; k< all; k++){
        x = checkExist(mails[queries[i].data.group_analyse_data.mids[k]].from);
        y = checkExist(mails[queries[i].data.group_analyse_data.mids[k]].to);
        if(x == y && people[x].q_time!=this_time){
            //printf("Send to his or her self. %d\n", count);
            count -= 2;
            //printf("Send to his or her self. %d\n", count);
            //printf("\n");
            continue;
        }
        unionByRank(x, y);
        //if(this_time == 3)
        //    printf("\n");
    }
    int ans[2] = {count, largest};
    api.answer(queries[i].id, ans, 2);
    //printf("%d %d\n", ans[0], ans[1]);

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
            testCount += 1;
        }
    }
    printf("%d\n", testCount);

    return 0;
}
