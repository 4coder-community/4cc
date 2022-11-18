
int i, j, count, is_prime;
int *primes;

primes = (int*)malloc(sizeof(int)*100);
count = 0;

for (i = 2; i < 100; ++i){
    is_prime = 1;
    for (j = 0; j < count; ++j){
        if (i % primes[j] == 0){
            is_prime = 0;
            break;
        }
    }
    if (is_prime){
        primes[count++] = i;
    }
}