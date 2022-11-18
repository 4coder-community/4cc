/*
 * YOUR INFO HERE!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define EULER_PROBLEM 14

#if EULER_PROBLEM == 1
#include "multiples_of_3_and_5.cpp"
#elif EULER_PROBLEM == 2
#include "even_fibonacci_numbers.cpp"
#elif EULER_PROBLEM == 3
#include "largest_prime_factor.cpp"
#elif EULER_PROBLEM == 4
#include "largest_palindrome_product.cpp"
#elif EULER_PROBLEM == 5
#include "smallest_multiple.cpp"
#elif EULER_PROBLEM == 6
#include "sum_square_difference.cpp"
#elif EULER_PROBLEM == 7
#include "prime_10001st.cpp"
#elif EULER_PROBLEM == 8
#include "largest_product_in_series.cpp"
#elif EULER_PROBLEM == 9
#include "special_pythagorean_triplet.cpp"
#elif EULER_PROBLEM == 10
#include "summation_of_primes.cpp"
#elif EULER_PROBLEM == 11
#include "largest_product_in_grid.cpp"
#elif EULER_PROBLEM == 12
#include "highly_divisible_triangular_number.cpp"
#elif EULER_PROBLEM == 13
#include "large_sum.cpp"
#elif EULER_PROBLEM == 14
#include "largest_collatz_sequence.cpp"
#endif

Euler_Result dummy_test(Euler_In in){
    Euler_Result result = {};
    return result;
}

#include <Windows.h>

int time(){
    int result = 0;
    
    LARGE_INTEGER t;
    if (QueryPerformanceCounter(&t)){
        result = (int)t.QuadPart;
    }
    
    return result;
}

int main(){
    Euler_In in;
    Euler_Result answer;
    int start, ms, dummy_ms;
    
    in = {};
    
    start = time();
    answer = dummy_test(in);
    dummy_ms = time() - start;
    start = time();
    answer = euler_main(in);
    ms = time() - start;
    
    printf("------RESULT------\n");
    euler_print(answer, in);
    
#ifdef EULER_CHECK
    printf("------CHECK-------\n");
    if (euler_check(answer, in)){
        printf("passed\n");
    }
    else{
        printf("failed\n");
    }
#endif
    
    printf("------TIME--------\n");
    printf("TOTAL:    %d\n", ms);
    printf("DUMMY:    %d\n", dummy_ms);
    printf("ADJUSTED: %d\n", ms - dummy_ms);
    
    return 0;
}
