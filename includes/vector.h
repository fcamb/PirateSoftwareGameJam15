#ifndef VECTOR_H
#define VECTOR_H

#ifndef VECTOR_IMPL
#define VECTOR_IMPL
#include <stdio.h>  
#include <stdlib.h>
#endif
typedef unsigned long long size_t;/* Prints vector data's value member -> must be named value */
#define vector(type) struct {type data; size_t length; size_t capacity;}
#define VectorType(type) struct type
#define OffsetOf(v,i) ((size_t)&(((v*)0)->i))

/* Usage */
// typedef vector(desired type ptr) name -- EX: typedef vector(int*) vector

#define VectorLen(v)            ((v)->length)
#define VectorCap(v)            ((v)->capacity)
#define Vector(v)               ((v)->data = CreateVector(sizeof(*((v)->data)),4), \
                                (v)->data ? (v)->length=0,(v)->capacity=4 : 0)
#define VectorCanGrow(v,n)      ((VectorLen((v))+(n) > VectorCap((v))) ? VectorGrow((v),(v)->data,VectorLen((v))+(n),VectorCap(v)*2) : 0)
#define VectorPush(v,val)       (VectorCanGrow((v),VectorLen((v))+1),(v)->data[v->length++]=(val)) 
#define VectorPushFront(v,val)  (VectorInsert(v,0,val))
#define VectorGrow(v,d,l,n)     ((d) = GrowVector((d), sizeof*((d)),(l),(n),&(v)->capacity))
#define VectorFree(v)           ((void) ((v)->data ? free((v).data) : (void)0),(v)->data=NULL)
#define VectorLast(v)           ((v)->data[(v)->length-1])
#define VectorPop(v)            ((v)->length--,ResizeCapacity(v))
#define VectorInsert(v,i,val)   (VectorValidIndex(v,i) ? VectorCanGrow((v),1),(v)->length+=1, \
                                 MemMove(&(v)->data[i+1],&(v)->data[i],sizeof*((v)->data)*(v)->length-1-(i)), \
                                 (v)->data[i]=(val) : 0) 
// vector,index,value,count
#define VectorInsertN(v,i,val,c)(VectorValidIndex(v,i) ?  VectorCanGrow((v),(c)), (v)->length+=(c), \
                                 MemMove(&((v)->data[(i)+(c)]),&((v)->data[i]),sizeof(*((v)->data))*(v)->length-(c)-(i)) : 0); \
                                 for(int j=i;j<((i)+(c));j++)(v)->data[j]=(val);
#define VectorDelete(v,i)       (VectorValidIndex(v,i) ? MemMove(&((v)->data)[i],&((v)->data)[i+1],sizeof(*((v)->data))*(v)->length-(i)-1), \
                                 (v)->length--, ResizeCapacity(v) : 0) 
#define VectorDeleteSwap(v,i)   ((v)->data[i] = VectorLast(v), (v)->length--, ResizeCapacity(v))
#define VectorDeleteN(v,i,n)    (VectorValidIndex((v),(i)) ? MemMove(&((v)->data)[i],&((v)->data)[(i)+(n)], sizeof(*((v)->data))*(v)->length-(i)-(n)), \
                                 (v)->length-=(n),ResizeCapacity(v) : 0)
#define VectorPopFront(v)       (MemMove(&((v)->data[0]),&((v)->data)[1], sizeof(*((v)->data))*(v)->length-1), (v)->length--,ResizeCapacity(v))
#define VectorEmpty(v)          (!(v)->length ? 1 : 0)
#define VectorItemAt(v,i)       (VectorValidIndex(v,i) ? (v)->data[i] : 0)
#define VectorValidIndex(v,i)   (((i)<=(v)->length-1)&&((i)>0||(i)==0))      
#define ResizeCapacity(v)       ((((v)->length < (v)->capacity/2)||((v)->length == (v)->capacity/4)) ? \
                                 GrowVector((v)->data,sizeof(*(v)->data), 0, (v)->capacity/2,&(v)->capacity), \
                                 (v)->capacity = (v)->capacity/2 : 0)

/* Double Pointer with member name "value"*/
#define VectorPtrSearch(data,val,len,idx)\
for(int i = 0; i < len; i++)                \
    if(data[i]->value == val) {             \
        index=i;                            \
        break;                              \
    } else {                                \
        index=-1;                           \
    }                                       
                                           
#define VectorPtrPrint(data,len,offset)\
for(int i = 0; i < len; i++){          \
    printf("%d,",data[i]->value);      \
    if((i+1)%offset==0)                \
        printf("\n");                  \
}

#define VectorPrint(data,len,offset)\
for(int i = 0; i < len; i++){       \
    printf("%d,", data[i]);         \
    if((i+1)%offset==0)             \
        printf("\n");               \
}

#define VectorSearch(data,val,len,idx)\
for (int i = 0; i < len; i++){        \
    if(data[i] == val) {              \
        index=i;                      \
        break;                        \
    } else {                          \
        index=-1;                     \
    }                                 \
}              

/* Prints vector data's value member -> must be named value */
#define VectorStructPrint(data,len,offset) \
for(int i = 0; i < len; i++){              \
    printf("%d,",data[i].value);           \
    if ((i+1)%offset==0)                   \
        printf("\n");                      \
}

static void*
CreateVector(size_t elemSize, size_t cap)
{
    void*b;
    b = malloc(elemSize*cap);
    if (b) {
        return b;
    }
    printf("Failed to allocated memory.\n");
    return NULL;
}

static void*
GrowVector(void* data, size_t elemSize, size_t addLen, size_t cap, void* actualCap)
{
    void*b;
    size_t* c = (size_t*)actualCap;
    if (addLen > cap) {
        cap = addLen;
        if (cap%2!=0) cap++;
        cap*=2;
    }
    
    *c = cap;

    b = realloc(data, elemSize * cap);
    if (b) {
        return b;
    }
    printf("Failed to reallocate memory.\n");
    return NULL;
}

/*https://github.com/gcc-mirror/gcc/blob/master/libgcc/memmove.c*/
static void*
MemMove(void* dst, const void* src, size_t size)
{
    
    char* d = (char*)dst;
    const char* s = (char*)src;
    // forwards
    if (d < s) {
        while (size--) {
            *d++ = *s++;
        }
    // backwards
    } else {
        char* lasts = (char*)s + (size-1);
        char* lastd = d + (size-1);
        while (size--) {
            *lastd-- = *lasts--;
        }
    }
    return dst;
}

static void*
MemCopy(void* dest, void* src, size_t size)
{
    char* d = (char*)dest;
    char* s = (char*)src;
    while(size--) {
        *d++ = *s++;
    }
    return dest;
}

#endif
