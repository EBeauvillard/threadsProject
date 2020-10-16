#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "thread.h"


void printvect(int n, int vect[]){
    for (int i = 0; i < n; i++){
        printf("%d ",vect[i]);
    }
    printf("\n");
}

void vectcp(int* v1, int*v2, int value){
    for (int i = 0; i < value; i++){
        v1[i] = v2[i];
    }
}


static void fuse(int vect1[], int n1, int vect2[], int n2, void* toreturn){
    int i = 0;
    int j = 0;
    int* result = malloc(sizeof(int) * (n1 + n2));

    while (i<n1 && j<n2){
        if (vect1[i]<vect2[j]){
            result[i+j] = vect1[i];
            i++;
        }else{
            result[i+j] = vect2[j];
            j++;
        }
    }

    while(i<n1){
        result[i+j] = vect1[i];
        i++;
    }
    while (j<n2){
        result[i+j] = vect2[j];
        j++;
    }

    vectcp((int *)toreturn,result,n1+n2);
    free (result);
}

static void * trifusion(void* param)
{
    thread_t th, th2;
    int err;
    void *res = NULL, *res2 = NULL;

    void** tab = (void**)param;
    unsigned long value = (unsigned long)(tab[0]);
    int* vect = (int*)(tab[1]);

    void *toreturn = malloc(value * sizeof(int));


    /* on passe un peu la main aux autres pour eviter de faire uniquement la partie gauche de l'arbre */
    thread_yield();


    if (value==1) {
        vectcp((int *)toreturn,vect, value);
        return toreturn;
    }

    unsigned long mid = value/2;

    void* param1[2] = {(void*)mid,(void*)(vect)};
    void* param2[2] = {(void*)(value-mid),(void*)(vect + mid)};
    void* pparam = NULL;
    void* pparam2 = NULL;
    pparam = param1;
    pparam2 = param2;


    err = thread_create(&th, trifusion, pparam);
    assert(!err);
    err = thread_create(&th2, trifusion, pparam2);
    assert(!err);

    err = thread_join(th, &res);
    assert(!err);
    err = thread_join(th2, &res2);
    assert(!err);

    fuse((int*)res,mid,(int*)res2,value-mid,toreturn);
    free (res);
    free(res2);
    return toreturn;
}


void place(int place,int* vect,int n){
    int c = 0;
    while (place>=0){
        if (vect[c] == 0)
            place --;
        c++;
    }
    vect[c-1]=n;
}

int main(int argc, char *argv[])
{
    unsigned long n;
    struct timeval tv1, tv2;
    double s;

    if (argc < 2) {
        printf("argument manquant: taille du tableau a trier\n");
        return -1;
    }

    n = atoi(argv[1]);


    srand(time(NULL));



    int* vect = malloc(sizeof(int) * n);
    for (int i = 0; i <n; i++){
        vect[i] = 0;
    }

    for (int i = 0; i <n; i++){
        int r = rand();
        place((r%(n-i)),vect,i);
    }

    printvect(n,vect);
    void* param[2] = {(void*)n,(void*)vect};
    void* value = NULL;
    value = param;
    gettimeofday(&tv1, NULL);
    int* res = (int*)trifusion((void *)value);
    gettimeofday(&tv2, NULL);
    s = (tv2.tv_sec-tv1.tv_sec) + (tv2.tv_usec-tv1.tv_usec) * 1e-6;

    printf("trifusion en %e s\n",s );
    printvect(n,res);


    free(res);
    free(vect);

    return 0;
}

