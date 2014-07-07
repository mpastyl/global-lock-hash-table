#include <stdio.h>
#include <stdlib.h>

// node of a list (bucket)
struct node_t{
    int value;
    struct node_t * next;
};
/*
struct SimpleList{
    struct node_t * Head;
}
*/

struct HashSet{
    //int length;
    struct node_t ** table;
    int capacity;
    int setSize;
    int lock;
};

void lock_set (struct HashSet * H){


    while (1){
        if (!H->lock){
            if(!__sync_lock_test_and_set(&(H->lock),1)) break;
        }
    }
}

void unlock_set(struct HashSet * H){

    H->lock = 0;
}



//search value in bucket;
int list_search(struct node_t * Head,int val){
    
    struct node_t * curr;
    
    curr=Head;
    while(curr){
        if(curr->value==val) return 1;
        curr=curr->next;
    }
    return 0;
}


//add value in bucket;
void list_add(struct HashSet * H, int key,int val){
    
    struct node_t * curr;
    struct node_t * next;
    struct node_t * node=(struct node_t *)malloc(sizeof(struct node_t));
    node->value=val;
    node->next=NULL;
    curr=H->table[key];
    if(curr==NULL){
        H->table[key]=node;
        return ;
    }
    while(curr->next){
        curr=curr->next;
        next=curr->next;
    }
    curr->next=node;
}




void initialize(struct HashSet * H, int capacity){
    
    int i;
    H->setSize=0;
    H->capacity=capacity;
    H->table = (struct node_t **)malloc(sizeof(struct node_t *)*capacity);
    for(i=0;i<capacity;i++){
        H->table[i]=NULL;
    }
}


void main(int argc,char * argv[]){

    struct HashSet * H=(struct HashSet *) malloc(sizeof(struct HashSet));
    initialize(H,10);
    list_add(H,5,7);
    list_add(H,5,8);
    int res=list_search(H->table[5],5);
    printf("res = %d \n",res);
    return;

    
}
