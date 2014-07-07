#include <stdio.h>
#include <stdlib.h>

// node of a list (bucket)
struct node_t{
    int value;
    int hash_code;
    struct node_t * next;
};


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
//NOTE: duplicate values are allowed...
void list_add(struct HashSet * H, int key,int val,int hash_code){
    
    struct node_t * curr;
    struct node_t * next;
    struct node_t * node=(struct node_t *)malloc(sizeof(struct node_t));
    /*node->value=val;
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
    */
    node->value=val;
    node->hash_code=hash_code;
    if(H->table[key]==NULL) node->next=NULL;
    else node->next=H->table[key];
    H->table[key]=node;
}


// delete from bucket. The fist value equal to val will be deleted
int list_delete(struct HashSet *H,int key,int val){
    
    struct node_t * curr;
    struct node_t * next;
    struct node_t * prev;

    curr=H->table[key];
    prev=curr;
    if((curr!=NULL)&&(curr->value==val)) {
        H->table[key]=curr->next;
        free(curr);
        return 1;
    }
    while(curr){
        if( curr->value==val){
            prev->next=curr->next;
            free(curr);
            return 1;
        }
        prev=curr;
        curr=curr->next;
    }
    return 0;
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


int policy(struct HashSet *H){
    return ((H->setSize/H->capacity) >4);
}

void resize(struct HashSet *);

int contains(struct HashSet *H,int hash_code, int val){
    
    lock_set(H);
    int bucket_index = hash_code % H->capacity;
    int res=list_search(H->table[bucket_index],val);
    unlock_set(H);
    return res;
}

//reentrant ==1 means we must not lock( we are calling from resize so we have already locked the data structure)
void add(struct HashSet *H,int hash_code, int val, int reentrant){
    
    if(!reentrant) lock_set(H);
    int bucket_index = hash_code % H->capacity;
    list_add(H,bucket_index,val,hash_code);
    H->setSize++;
    if(!reentrant) unlock_set(H);
    if (policy(H)) resize(H);
}

int delete(struct HashSet *H,int hash_code, int val){
    
    lock_set(H);
    int bucket_index =  hash_code % H->capacity;
    int res=list_delete(H,bucket_index,val);
    H->setSize--;
    unlock_set(H);
    return res;
}


void resize(struct HashSet *H){
    
    int i;
    struct node_t * curr;
    int old_capacity = H->capacity;
    lock_set(H);
    if(old_capacity!=H->capacity){
        unlock_set(H);
        return; //somebody beat us to it
    }
    int new_capacity =  old_capacity * 2;
    H->capacity =  new_capacity;

    struct node_t ** old_table = H->table;
    H->setSize=0;
    H->table = (struct node_t **)malloc(sizeof(struct node_t *)*new_capacity);
    for(i=0;i<new_capacity;i++){
        H->table[i]=NULL;
    }
    for(i=0;i<old_capacity;i++){
        
        curr=old_table[i];
        while(curr){
                int val = curr->value;
                int hash_code = curr->hash_code;
                //int bucket_index= hash_code % new_capacity;
                add(H,hash_code,val,1);
                curr=curr->next;
        }
    }
    free(old_table);
    unlock_set(H);
}

void print_set(struct HashSet * H){
    
    int i;
    for(i=0;i<H->capacity;i++){
        
        struct node_t * curr=H->table[i];
        while(curr){
            printf("(%d) ",curr->value);
            curr=curr->next;
        }
        printf("--\n");
    }
}

void main(int argc,char * argv[]){

    struct HashSet * H=(struct HashSet *) malloc(sizeof(struct HashSet));
    initialize(H,10);
    /*list_add(H,5,7);
    list_add(H,5,8);
    int res=list_search(H->table[5],5);
    printf("res = %d \n",res);
    */

    /*add(H,4,10);
    add(H,8,9);
    add(H,8,7);
    add(H,4,5);
    add(H,3,2);
    add(H,0,1);
    add(H,9,12);
    add(H,4,9);
    */

    /*printf("delete res %d \n",delete(H,3,3));
    printf("delete res %d \n",delete(H,3,2));
    printf("delete res %d \n",delete(H,8,7));
    printf("delete res %d \n",delete(H,2,3));

    printf(" res %d \n",contains(H,8,7));//n
    printf(" res %d \n",contains(H,3,2));//n
    printf(" res %d \n",contains(H,8,9));//y
    printf(" res %d \n",contains(H,2,3));//n
    */
    srand(time(NULL));
    int i,j;
    #pragma omp parallel for num_threads(5) shared(H) private(i,j)
    for(j=0;j<5;j++){
        for(i=0;i<10;i++){
            add(H,i+j*10,i+j*10,0);
            //add(H,rand(),i,0);
        }
    }
    print_set(H);
    return;

    
}
