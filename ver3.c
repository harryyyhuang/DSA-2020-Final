#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define min(a,b) (a<=b?a:b)
#define radix 39
#define debug 0

#define explimit 5
int tableSize=(1<<25)+5;
float reward=0;
int querytimes=0;
int n_cap[10000][10000];
int token_index[10000][10000];
// The testdata only contains the first 100 mails (mail1 ~ mail100)
// and 2000 queries for you to debug.
typedef struct node node;
struct node{
	char* str;
	int id;
	node* next;
};

node** chainingTable;
int hash_char[256];
void hash_init(){
    for(int c=0;c<256;c++){
        int val=-1;
        if('0'<=c && c<='9'){
            val=c-'0';
        }else if('a'<=c && c<='z'){
            val=c-'a'+10;
        }else if('A'<=c && c<='Z'){
            val=c-'A'+10;
        }

        hash_char[c]=val;
    }
}
int hash(char* s,int id){
	int val=(id+1)*radix-1;
	for (int i=0;s[i]!='\0';i++){
		val*=radix;
		int temp=hash_char[s[i]];
		if(temp!=-1){
			val+=temp;
		}else{
			val/=radix;
			return val;
		}
		val%=tableSize;
	}
	return val;

}
bool compare(char* a,char* b){
    int i=0;
    while(true){
        if(a[i]!='\0' && b[i]!='\0'){
            if(hash_char[a[i]]!=hash_char[b[i]]){
                return false;
            }
        }else if(a[i]=='\0' && b[i]=='\0' ) {
            return true;
        }else{
            return false;
        }
        i++;
    }
    return true;
}
node* map_count(char* s,int id){
	node* temp=chainingTable[hash(s,id)];
	
	while(temp){
		if(compare(temp->str,s) && temp->id==id){
			return temp;
		}else{
			temp=temp->next;
		}
	}
	return NULL; 
}
char* str_copy(char* s,int len){
	char* str=(char*) calloc(len+1,sizeof(char));
	for( int l=0;l<len;l++){
		str[l]=s[l];
	}
	str[len]='\0';
	return str;
}
int map_append(char* s,int id){
	node* temp=map_count(s,id);
	if(temp!=NULL){
		return 0;
	}
	else {
		node** temp=&chainingTable[hash(s,id)];
		int len=strlen(s);
		char* str =str_copy(s,len);
		if(*temp==NULL){
			*temp= (node*) malloc(sizeof(node));
		}else{
			while((*temp)->next){
				temp=&((*temp)->next);
			}
			(*temp)->next=(node*) malloc(sizeof(node));
			temp=&((*temp)->next);
		}
		(*temp)->str=str;
		(*temp)->id=id;
		(*temp)->next=NULL;
	}
	return 1;

}
int n_mails, n_queries;
mail *mails;
query *queries;
int* setSize;
char*** tokenSets;

void preparation(){
	
	chainingTable=(node**) calloc(tableSize,sizeof(node*));
	tokenSets=(char***) calloc(n_mails,sizeof(char**));
	setSize=(int*) calloc(n_mails,sizeof(int));
    hash_init();
    memset(n_cap,0,sizeof(n_cap));
    memset(token_index,0,sizeof(n_cap));
	int len;
	for (int j=0;j<n_mails;j++){
		node* head=(node*) malloc(sizeof(node));
		head->next=NULL;
		node* temp=head;
		char* subject=mails[j].subject;
		char* content=mails[j].content;
		int i=0;
        int k=0;
        while(true){
            if(hash_char[subject[k]]==-1){
				len=k-i;
				if(len>0){
					char* str=str_copy(subject+i,len);
					
					if(map_append(str,j)){
						temp->next=(node*) malloc(sizeof(node));
						temp=temp->next;
						temp->str=str;
						temp->next=NULL;
						setSize[j]++;
					}else{
						free(str);
					}
				}
				i=k+1;
			}
            if(subject[k]=='\0') break;
            k++;

        }
        /*
		for(int k=0;k<subject_len;k++){
			if(hash_char[subject[k]]==-1){
				
				len=k-i;
				if(len>0){
					char* str=str_copy(subject+i,len);
					
					if(map_append(str,j)){
						temp->next=(node*) malloc(sizeof(node));
						temp=temp->next;
						temp->str=str;
						temp->next=NULL;
						setSize[j]++;
					}else{
						free(str);
					}
				}
				i=k+1;
			}
		}
        */
		k=i=0;
        while(true){
            if(hash_char[content[k]]==-1){
				len=k-i;
				if(len>0){
					char* str=str_copy(content+i,len);
					
					if(map_append(str,j)){
						temp->next=(node*) malloc(sizeof(node));
						temp=temp->next;
						temp->str=str;
						temp->next=NULL;
						setSize[j]++;
					}else{
						free(str);
					}
				}
				i=k+1;
			}
            if(content[k]=='\0') break;
            k++;

        }
        /*
		for(int k=0;k<content_len;k++){
			if(hash_char[content[k]]==-1){
				len=k-i;
				if(len>0){
					char* str=str_copy(content+i,len);
					
					if(map_append(str,j)){
						temp->next=(node*) malloc(sizeof(node));
						temp=temp->next;
						temp->str=str;
						temp->next=NULL;
						setSize[j]++;
					}else{
						free(str);
					}
				}
				i=k+1;
			}
		}
        */
		temp=head->next;
		tokenSets[j]=(char**) malloc(setSize[j]*sizeof(char*));
		for(int k=0;k<setSize[j];k++){
			tokenSets[j][k]=temp->str;
			free(temp);
			temp=temp->next;
		}
	}

}

char* symbols;
bool* boolvals;
int s_top;
int b_top;
int operationRank(char c){
	switch (c)
	{
		case '|':
			return 2;
		case '&':
			return 3;
		case '!':
			return 4;
		case '(':
		case ')':
			return 1;
	}
	return 0;
}
void calculation(){
	switch (symbols[s_top])
	{
	case '!':
		boolvals[b_top]=!boolvals[b_top];
		break;
	case '|':
		boolvals[b_top-1]|=boolvals[b_top];
		b_top--;
		break ;
	case '&':
		boolvals[b_top-1]&=boolvals[b_top];
		b_top--;
		break;
	}
	s_top--;
	return;
}
node* findString(node* head,char* str){
    node* temp=head->next;
    while(temp){
        if(compare(temp->str,str)){
            return temp;
        }
        temp=temp->next;
    }
    return NULL;
}

int findKmap(bool** kmap, char*** tokenlist, char* expre){
    int n_variables=0;
    int i=0;
    char* explist[100];
    int list_len=0;
    node* head=(node*)calloc(1,sizeof(node));
    node* curr=head;
    curr->next=NULL;
    while(expre[i]!='\0'){
        if(hash_char[expre[i]]!=-1){
            int start=i;
            while(expre[i]!='\0' && hash_char[expre[i]]!=-1)i++;
            
            int str_len=i-start;
            char* str=str_copy(expre+start,str_len);
            
            node* temp=findString(head,str);
            
            char* variable=(char*) calloc(2,sizeof(char));
            explist[list_len++]=variable;;
            
            if(temp==NULL){
                if(n_variables>=explimit){
                    free(str);
                    free(head);
                    temp=head->next;
                    while(temp){
                        free(temp->str);
                        free(temp);
                        temp=temp->next;
                        
                    }
                    for(int i=0;i<list_len;i++){
                        free(explist[i]);
                    }
                    return n_variables+1;
                }
                curr->next=(node*)calloc(1,sizeof(node));
                curr=curr->next;
                curr->str=str;
                curr->next=NULL;
                curr->id=n_variables;        
                variable[0]=n_variables++ +'0';
            }else{
                free(str);
                variable[0]=temp->id+'0';
            }
        }else{
            char* symbol=(char*) calloc(2,sizeof(char));
            symbol[0]=expre[i++];
            explist[list_len++]=symbol;
        }

    }

    if(n_variables<=explimit){
        int times=1<<n_variables;
        *tokenlist=(char**) calloc(n_variables,sizeof(char*));
        node* curr=head->next;
        for (int i=0;i<n_variables;i++){
            (*tokenlist)[i]=curr->str;
            curr=curr->next;
        }
        curr=head->next;
        *kmap=(bool*) calloc(times,sizeof(bool));
        
        symbols=(char*) calloc(list_len,sizeof(char));
        boolvals=(bool*) calloc(list_len,sizeof(bool));
        for(int i=0;i<times;i++){
            s_top=-1;
            b_top=-1;
            int j=0;
            while(j<list_len){
                char c=explist[j][0];
                if(hash_char[c]!=-1){
                    boolvals[++b_top]=(i&(1<<(c-'0')));
                }else{
                    if(c=='('){
                        symbols[++s_top]=c;
                    }else if(c==')'){	
                        while(s_top>-1 && symbols[s_top]!='('){
                            calculation();
                        }
                        s_top--;
                    }else{
                        while(s_top>-1 && operationRank(symbols[s_top])>=operationRank(c)){
                            calculation();
                        }
                        symbols[++s_top]=c;
                    }   
                }
                j++;
            }
            while(s_top>-1) calculation();
            if(boolvals[0]){
                (*kmap)[i]=1;
            }

        }
        free(symbols);
	    free(boolvals);

        
    }
    while(head){
        //free(curr->str);
        free(head);
        head=head->next;
    }
    for(int i=0;i<list_len;i++){
        free(explist[i]);
    }
    return n_variables;
}


void expression_matching(int i){
    
    if(queries[i].reward<1) return;
	
    char* expre=queries[i].data.expression_match_data.expression;
	if(strlen(expre)>50) return;

	int exptokens;
	char** explist;
    
    bool* kmap;
    char** tokenlist;
    int token_len=findKmap(&kmap,&tokenlist,expre);
    if(token_len>explimit) return;
	reward+=queries[i].reward;
    querytimes++;
	int len=0;
	int result[n_mails];
    for(int id=0;id<n_mails;id++){
        int kmap_index=0;
        for(int i=0;i<token_len;i++){
            if(map_count(tokenlist[i],id)) kmap_index+=(1<<i);
        }
		if(kmap[kmap_index]){
			result[len++]=id;
		}

	}

	if(debug)	printf("querytimes=%d,reward=%f\n",querytimes,reward);
	api.answer(queries[i].id, result, len);
	free(kmap);
	free(tokenlist);
}
float rate(int a,int b, int x){
    return (((float)x )/((float) a+b-x));
}
void find_similarity(int i){
    
    int mid=queries[i].data.find_similar_data.mid;
    float threshold=queries[i].data.find_similar_data.threshold;
	if(setSize[mid]>20 || queries[i].reward<60) return;
	//if(setSize[mid]>14|| (queries[i].reward/ (setSize[mid]*threshold))<100  ) return ;
    if(debug) printf("cp=%f\n",(queries[i].reward/ (setSize[mid]*threshold)));
	//printf("%f,%d,%f\n",threshold,setSize[mid],threshold*setSize[mid]);
	reward+=queries[i].reward;
    querytimes++;
    int len=0;
    int result[n_mails];
	int count=0;
    for(int id=0;id<n_mails;id++){
        
        if(id==mid ||
        rate(setSize[mid],setSize[id],min(setSize[mid],setSize[id]))<=threshold ){
            continue;
        }
        
        else if(setSize[mid]<setSize[id]|| (setSize[mid]==setSize[id] && mid<id)){
            int j=token_index[mid][id];
            for(;j<setSize[mid];j++){
                if(rate(setSize[mid],setSize[id],n_cap[mid][id])>threshold)break;
                if(map_count(tokenSets[mid][j],id)){
                    n_cap[id][mid]=++n_cap[mid][id];
                }
            }
            token_index[id][mid]=token_index[mid][id]=j;
        }else{
            int j=token_index[id][mid];
            for(;j<setSize[id];j++){
                if(rate(setSize[mid],setSize[id],n_cap[id][mid])>threshold)break;
                if(map_count(tokenSets[id][j],mid)){
                    n_cap[mid][id]=++n_cap[id][mid];
                }
            }
            token_index[mid][id]=token_index[id][mid]=j;
        }
        if(rate(setSize[mid],setSize[id],n_cap[mid][id])>threshold){
            result[len++]=id;
        }


/*

        int n_cap=0; 
        if(setSize[mid]<=setSize[id]){
            for(int j=0;j<setSize[mid];j++){
            
                if(map_count(tokenSets[mid][j],id)){
                    n_cap++;
                    
                }
                if(rate(setSize[mid],setSize[id],n_cap)>threshold){
                    result[len++]=id;
                    
                    break;
                }
            }
        }else{
            for(int j=0;j<setSize[id];j++){
                if(map_count(tokenSets[id][j],mid)){
                    n_cap++;
                    
                }
                if(rate(setSize[mid],setSize[id],n_cap)>threshold){
                    result[len++]=id;
                    
                    break;
                }
            }
        }
     */   
    }
    
	/*
	for(int i=0;i<len;i++){
		printf("%d ",result[i]);
	}
	printf("\n");
	*/
	if(debug)printf("queriestimes=%d,reward=%f\n",querytimes,reward);
	api.answer(queries[i].id, result, len);
}


void printdata(int i){
	printf("TokenSize=%d\n",setSize[i]);
}
int main(void) {
	api.init(&n_mails, &n_queries, &mails, &queries);
	preparation();
    /*
    for(int i=0;i<10000;i++){
        for(int j=0;j<10000;j++){
            if(token_index[i][j]!=0)
                printf("index=%d\n",token_index[i][j]);
            if(n_cap[i][j]!=0)
                printf("n_cap=%d\n",n_cap[i][j]);
        }
    }
    */
    /*
    int n_lenlarge=0;
    int gap=12;
    for(int i=0;i<tableSize;i++){
        int len=0;
        node* temp=chainingTable[i];
        while(temp){
            len++;
            temp=temp->next;
        }
        if(len>gap){
            n_lenlarge++;
            printf("len (table[%d])=%d\n",i,len);
        }
    }
    printf("number of len>%d is %d\n",gap,n_lenlarge);
    return 0;

*/

	/*
	for(int i=0;i<n_mails;i++){
		printf("%d\n",setSize[i]);
	}
	*/
/*
    int smallSets=0;
    int smallSize=50;

    for(int i=0;i<n_mails;i++){
        if(setSize[i]<smallSize){
            smallSets++;
        }
    }
    printf("smallsize=%d,SmallSets=%d\n",smallSize,smallSets);
    return 0;
    */
	int index;
	//printdata(index);
		//!((clementson)|(!amenities)&(!montitvo));
	int largereward=0;
	for(int i = 0; i < n_queries; i++){
		if (queries[i].type==find_similar){
            
            find_similarity(i);
        }
	}
    for(int i = 0; i < n_queries; i++){
		if(queries[i].type == expression_match)
		  	expression_matching(i);
	}
    //printf("large reward=%d\n",largereward);
	
  return 0;
}

/*
 gcc -o ver1 ver1.c -std=c11
 
 ./ver1 <../testdata/test.in
 */

