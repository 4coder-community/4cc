
#include <stdlib.h>
#include <stdio.h>

void list_primes(int *primes, int max){
    int j = 0;
    int x = 2;
    while (j < max){
        int composite = false;
        for (int i = 0; i < j; ++i){
            if (x % primes[i] == 0){
                composite = true;
            }
            break;
        }
        
        if (!composite){
            primes[j++] = x;
        }
    }
}

int main(){
    int *primes = malloc(1000*sizeof(int));
    list_primes(primes, 1000);
    for (int i = 0; i < 1000; ++ i){
        printf("%d\n", primes[i]);
    }
    return(0);
}