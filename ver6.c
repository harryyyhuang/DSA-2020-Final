#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define min(a,b) (a<=b?a:b)
#define radix 39
#define debug 1
#define explimit 6
int tableSize=(1<<25)+5;
float reward=0;
int querytimes=0;
short* n_cap;
short* token_index;
// The testdata only contains the first 100 mails (mail1 ~ mail100)
// and 2000 queries for you to debug.
typedef struct node node;
typedef struct token token;
struct node{
	char* str;
    int len;
	int id;
    int val;
    int count;
    int index;
	node* next;
};
struct token{
    char* str;
    int len;
    int hash_val;
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
int hash_int(int key,int id){
    return (key+id+1)%tableSize;
}
int hash(char* s,int len,int id){
	long long val=0;
	for (int i=0;i<len;i++){
		val*=radix;
		if(hash_char[s[i]]!=-1){
			val+=hash_char[s[i]];
		}else{
			val/=radix;
			return val;
		}
		val%=tableSize;
	}
    val=(val*10000)%tableSize;
    if(id>-1){
        val=val+id+1;
        val%=tableSize;
    }
	return val;

}
bool compare(char* a,int lena,char* b,int lenb){
    if(lena!=lenb) return false;
    else{
        for(int i=0;i<lena;i++){
            if(hash_char[a[i]]!=hash_char[b[i]]){
                return false;
            }
        }
    }
    return true;

}

node* map_count(char* s,int len,int hash_val,int id){
	node* temp=chainingTable[hash_int(hash_val,id)];
	
	while(temp){
		if(compare(temp->str,temp->len,s,len) && temp->id==id){
			return temp;
		}else{
			temp=temp->next;
		}
	}
	return NULL; 
}
node* map_count_token(token* t,int id){
    
	return map_count(t->str,t->len,t->hash_val,id); 


}

char* str_copy(char* s,int len){
	char* str=(char*) calloc(len+1,sizeof(char));
	for( int l=0;l<len;l++){
		str[l]=s[l];
	}
	str[len]='\0';
	return str;
}

int map_append(char* s,int len,int hash_val,int id){
	node* temp=map_count(s,len,hash_val,id);
	if(temp!=NULL){
        temp->count++;
		return 0;
	}
	else {
		node** temp=&chainingTable[hash_int(hash_val,id)];
		if(*temp==NULL){
			*temp= (node*) malloc(sizeof(node));
		}else{
			while((*temp)->next){
				temp=&((*temp)->next);
			}
			(*temp)->next=(node*) malloc(sizeof(node));
			temp=&((*temp)->next);
		}
		(*temp)->str=s;
		(*temp)->id=id;
        (*temp)->len=len;
        (*temp)->count=1;
        (*temp)->index=-1;
		(*temp)->next=NULL;
	}
	return 1;

}
int map_append_(char* s,int len,int val,int id){
    map_append(s,len,val,-1);
    return map_append(s,len,val,id);
}
int n_mails, n_queries;
mail *mails;
query *queries;
short* setSize;
short* n_nonrepat;
bool* sets;
int** tokenIndex;
char** wordlist;



void preparation(){
    
	chainingTable=(node**) calloc(tableSize,sizeof(node*));
    sets=(bool*) calloc(1000000000,sizeof(bool));
    tokenIndex=(int**) malloc(n_mails*sizeof(int*));
	setSize=(short*) calloc(n_mails,sizeof(short));
    n_nonrepat=(short*) calloc(n_mails,sizeof(short));

    //wordlist=(char**) calloc(200000,sizeof(char*));
    hash_init();
    
    n_cap=(short*) calloc(10000*10000,sizeof(short));

    token_index=(short*) calloc(10000*10000,sizeof(short));


    char* strs[8];//I read the paragraph on http wikipedia org
    int start0[8]={605,607,612,616,626,629,636,646};
    int slen[8]={1,4,3,9,2,4,9,3};
    for(int k=0;k<8;k++){
        strs[k]=mails[0].content+start0[k];
    }
    
    
    
    for(int k=0;k<8;k++){
        int hash_val=hash(strs[k],slen[k],-1);
        for(int id=0;id<n_mails;id++){  
            map_append(strs[k],slen[k],hash_val,id);     
        }
    }
    


	int len;
    node lists[n_mails];
    int nodeindex=0;
    
	for (int j=0;j<n_mails;j++){
		node* temp=&lists[j];
		temp->next=NULL;

		char* subject=mails[j].subject;
		char* content=mails[j].content;
		int i=0;
        int k=0;
        while(true){
            if(hash_char[subject[k]]==-1){
				len=k-i;
				if(len>0){
					char* str=subject+i;
                    int hash_val=hash(str,len,-1);
					if(map_append_(str,len,hash_val,j)){
						temp->next=(node*) malloc(sizeof(node)+1);
						temp=temp->next;
                        temp->val=hash_val;
						temp->str=str;
                        temp->len=len;
						temp->next=NULL;
						setSize[j]++;

					}
				}
				i=k+1;
			}
            if(subject[k]=='\0') break;
            k++;

        }
		k=i=0;
        while(true){
            if(hash_char[content[k]]==-1){
				len=k-i;
				if(len>0){
					char* str=content+i;
					int hash_val=hash(str,len,-1);
					if(map_append_(str,len,hash_val,j)){
						temp->next=(node*) malloc(sizeof(node)+1);

						temp=temp->next;
                        temp->val=hash_val;
						temp->str=str;
                        temp->len=len;
						temp->next=NULL;

						setSize[j]++;
                        
					}
				}
				i=k+1;
			}
            if(content[k]=='\0') break;
            k++;

        }

		
	}
    int word_index=0;
    for(int j=0;j<n_mails;j++){
        node* temp=lists[j].next;
        node* next=NULL;
        tokenIndex[j]=(int*) malloc((setSize[j])*sizeof(int));
        int index=0;
        int pos=j*100000;
        for(int k=0;k<setSize[j];k++){
                node* word=map_count(temp->str,temp->len,temp->val,-1);
            if(word->count!=1){
                if(word->index!=-1){
                    sets[pos+word->index]=true;
                }
                else{
                 //   wordlist[word_index]=str_copy(word->str,word->len);
                    word->index=word_index;
                    sets[pos+word_index++]=true;

                }
                tokenIndex[j][index]=word->index;
                index++;
            }else{
                n_nonrepat[j]++;
            }
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

node* findString(node* head,int val,char* str,int len){
    node* temp=head->next;
    while(temp){
        if(temp->val==val && compare(temp->str,temp->len,str,len)){
            return temp;
        }
        temp=temp->next;
    }
    return NULL;
}

int findKmap(bool** kmap, token** tokenlist, char* expre){
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
            char* str=expre+start;
            int val=hash(str,str_len,-1);
            node* temp=findString(head,val,str,str_len);
            
            char* variable=(char*) calloc(2,sizeof(char));
            explist[list_len++]=variable;;
            
            if(temp==NULL){
                if(n_variables>=explimit){
                    /*
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
                    */
                    return n_variables+1;
                }
                curr->next=(node*)calloc(1,sizeof(node));
                curr=curr->next;
                curr->str=str;
                curr->val=val;
                curr->len=str_len;
                curr->next=NULL;
                curr->id=n_variables;        
                variable[0]=n_variables++ +'0';
            }else{
                variable[0]=temp->id+'0';
            }
        }else{
            char* symbol=(char*) calloc(1,sizeof(char));
            symbol[0]=expre[i++];
            explist[list_len++]=symbol;
        }

    }

    if(n_variables<=explimit){
        int times=1<<n_variables;
        *tokenlist=(token*) calloc(n_variables,sizeof(token));
        node* curr=head->next;
        for (int i=0;i<n_variables;i++){
            (*tokenlist)[i].str=curr->str;
            (*tokenlist)[i].hash_val=curr->val;
            (*tokenlist)[i].len=curr->len;
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
        /*
        free(symbols);
	    free(boolvals);
*/
        
    }
    /*
    while(head){
        free(curr->str);
        free(head);
        head=head->next;
    }
    for(int i=0;i<list_len;i++){
        free(explist[i]);
    }
    */
    return n_variables;
}


void expression_matching(int i){
    
    if(queries[i].reward<0.8) return;
	
    char* expre=queries[i].data.expression_match_data.expression;
	if(strlen(expre)>70) return;

	int exptokens;
	char** explist;
    
    bool* kmap;
    token* tokenlist;
    int token_len=findKmap(&kmap,&tokenlist,expre);
    if(token_len>explimit) return;
	reward+=queries[i].reward;
    querytimes++;
	int len=0;
	int result[n_mails];
    for(int id=0;id<n_mails;id++){
        int kmap_index=0;
        for(int i=0;i<token_len;i++){
            if(map_count_token(&tokenlist[i],id)) kmap_index+=(1<<i);            
        }
		if(kmap[kmap_index]){
			result[len++]=id;
		}

	}

	if(debug)	printf("querytimes=%d,reward=%f\n",querytimes,reward);
	api.answer(queries[i].id, result, len);
	/*
    free(kmap);
	free(tokenlist);
    */
}

float rate(int a,int b, int x){
    return (((float)(x+8) )/ (a+b+8-x));
}
void find_similarity(int i){
    
    int mid=queries[i].data.find_similar_data.mid;
    float threshold=queries[i].data.find_similar_data.threshold;
	if(setSize[mid]-n_nonrepat[mid]>35 || queries[i].reward<80) return;
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
        int index;
        int setindex;
        if(setSize[mid]<setSize[id]|| (setSize[mid]==setSize[id] && mid<id)){
            index=id*10000+mid;
            setindex=id*100000;
            int j=token_index[index];
            for(;j<setSize[mid]-n_nonrepat[mid];j++){
                if(rate(setSize[mid],setSize[id],n_cap[index])>threshold)break;
                if(sets[setindex+tokenIndex[mid][j]]){
                    n_cap[index]++;
                }
            }
            token_index[index]=j;
        
        }else{
            index=mid*10000+id;
            setindex=mid*100000;
            int j=token_index[index];
            for(;j<setSize[id]-n_nonrepat[id];j++){
                if(rate(setSize[mid],setSize[id],n_cap[index])>threshold)break;
                if(sets[setindex+tokenIndex[id][j]]){
                    n_cap[index]++;
                }
            }
            token_index[index]=j;
        }
        if(rate(setSize[mid],setSize[id],n_cap[index])>threshold){
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

void measureTokenSize(){
    if(!debug) return;
    int n_lenlarge=0;
    int gap=2;
    int total=0;
    int countt=0;
    for(int i=0;i<tableSize;i++){
        int len=0;
        node* temp=chainingTable[i];
        while(temp){
            if(temp->id==-1)countt++;
            total++;
            len++;
            temp=temp->next;
        }
        if(len>gap){
            n_lenlarge++;
            printf("len (table[%d])=%d\n",i,len);
        }
    }
    printf("number of len>%d is %d\n",gap,n_lenlarge);
    printf("total token size=%d\n",total);
    printf("num of id=-1 is %d\n",countt);
}
void printdata(int i){
	printf("TokenSize=%d\n",setSize[i]);
}
void printString(char* str, int len){
    for(int i=0;i<len;i++){
        putchar(str[i]);
    }
}
int main(void) {
    
    api.init(&n_mails, &n_queries, &mails, &queries);
   // int len=strlen(mails[0].content);
    //printf("len=%d,%s\n",len,mails[0].content+len-21);
    
	preparation();
    /*
    for(int id=0;id<n_mails;id++){
        for(int i=0;i<setSize[id]-n_nonrepat[id];i++){
            token* t=&tokenSets[id][i];
            char* str=wordlist[tokenIndex[id][i]];
            if(sets[id*10000+tokenIndex[id][i]]==0){
                printf("id=%d,i=%d,index=%d,sets[%d][%d]=%d\n",id,i,tokenIndex[id][i],id,tokenIndex[id][i],
                sets[id*10000+tokenIndex[id][i]]);
            }
            if(compare(t->str,t->len,str,strlen(str))==0){
                printf("id=%d\n",id);
                printString(t->str,t->len);
                putchar(' ');
                puts(str);
                putchar('\n');

            }
        }
    }
    */
    
    
   //129
    /*
    hash_init();
    char exp[]="!((clementson)|(!amenities)&(!montitvo))";
    bool* kmap;
    token* tokens;
    int len=findKmap(&kmap,&tokens,exp);
    printf("len=%d\n",len);
    for(int i=0;i<len;i++){
        for(int j=0;j<tokens[i].len;j++){
            char c=(tokens[i].str)[j];
            printf("%c",c);
        }
        printf(" ");
    }
    printf("\n");
    for(int i=0;i<(1<<len);i++){
        printf("%d:",i);
        for(int j=0;j<len;j++){
            printf("%d ",(bool)(i&(1<<j)));
        }
        printf("| %d\n",kmap[i]);
    }
    printf("\n");
    expression_matching(1);
    return 0;
    */
    /*
    int total=0;
    for(int i=0;i<n_mails;i++){
        total+=n_nonrepat[i];
    }
    printf("%d\n",total);
    printf("\n");
    return 0;
    */
	for(int i = 0; i < n_queries; i++){
		if (queries[i].type==find_similar){
            

            find_similarity(i);
        }
	}
    
    
    
    /*
    for(int i = 0; i < n_queries; i++){
		if(queries[i].type == expression_match)
		  	expression_matching(i);
	}
    */
    
    return 0;
}

/*
 gcc -o ver1 ver1.c -std=c11
 
 ./ver1 <../testdata/test.in
 */

