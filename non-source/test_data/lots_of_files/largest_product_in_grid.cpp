/*
 * YOUR INFO HERE!
 */

// FINAL VERSION:

int largest_product_in_series(int *series, int size, int prod_count, int stride = 1){
    int c=0,x=1,b=0,d,i;
    for (i = 0; i < size; ++i){
        d = series[(i)*stride];
        if (d == 0){
            c = 0;
            x = 1;
        }
        else{
            if (c < prod_count){
                x *= d;
                ++c;
                if (c == prod_count && x > b) b = x;
            }
            else{
                x /= series[(i - prod_count)*stride];
                x *= d;
                if (x > b) b = x;
            }
        }
        
    }
    return b;
}

#ifdef EULER_PROBLEM
// BUILDING AREA:

struct Euler_In{};

struct Euler_Result{
    int product;
};

static int GRID[] = {
     8,  2, 22, 97, 38, 15,  0, 40,  0, 75,  4,  5,  7, 78, 52, 12, 50, 77, 91,  8,
    49, 49, 99, 40, 17, 81, 18, 57, 60, 87, 17, 40, 98, 43, 69, 48,  4, 56, 62,  0,
    81, 49, 31, 73, 55, 79, 14, 29, 93, 71, 40, 67, 53, 88, 30,  3, 49, 13, 36, 65,
    52, 70, 95, 23,  4, 60, 11, 42, 69, 24, 68, 56,  1, 32, 56, 71, 37,  2, 36, 91,
    22, 31, 16, 71, 51, 67, 63, 89, 41, 92, 36, 54, 22, 40, 40, 28, 66, 33, 13, 80,
    24, 47, 32, 60, 99,  3, 45,  2, 44, 75, 33, 53, 78, 36, 84, 20, 35, 17, 12, 50,
    32, 98, 81, 28, 64, 23, 67, 10, 26, 38, 40, 67, 59, 54, 70, 66, 18, 38, 64, 70,
    67, 26, 20, 68,  2, 62, 12, 20, 95, 63, 94, 39, 63,  8, 40, 91, 66, 49, 94, 21,
    24, 55, 58,  5, 66, 73, 99, 26, 97, 17, 78, 78, 96, 83, 14, 88, 34, 89, 63, 72,
    21, 36, 23,  9, 75,  0, 76, 44, 20, 45, 35, 14,  0, 61, 33, 97, 34, 31, 33, 95,
    78, 17, 53, 28, 22, 75, 31, 67, 15, 94,  3, 80,  4, 62, 16, 14,  9, 53, 56, 92,
    16, 39,  5, 42, 96, 35, 31, 47, 55, 58, 88, 24,  0, 17, 54, 24, 36, 29, 85, 57,
    86, 56,  0, 48, 35, 71, 89,  7,  5, 44, 44, 37, 44, 60, 21, 58, 51, 54, 17, 58,
    19, 80, 81, 68,  5, 94, 47, 69, 28, 73, 92, 13, 86, 52, 17, 77,  4, 89, 55, 40,
     4, 52,  8, 83, 97, 35, 99, 16,  7, 97, 57, 32, 16, 26, 26, 79, 33, 27, 98, 66,
    88, 36, 68, 87, 57, 62, 20, 72,  3, 46, 33, 67, 46, 55, 12, 32, 63, 93, 53, 69,
     4, 42, 16, 73, 38, 25, 39, 11, 24, 94, 72, 18,  8, 46, 29, 32, 40, 62, 76, 36,
    20, 69, 36, 41, 72, 30, 23, 88, 34, 62, 99, 69, 82, 67, 59, 85, 74,  4, 36, 16,
    20, 73, 35, 29, 78, 31, 90,  1, 74, 31, 49, 71, 48, 86, 81, 16, 23, 57,  5, 54,
     1, 70, 54, 71, 83, 51, 54, 69, 16, 92, 33, 48, 61, 43, 52,  1, 89, 19, 67, 48
};

static const int W = 20;
static const int H = 20;

#define GET_I(x,y) ((x)+W*(y))

Euler_Result euler_main(Euler_In in){
#if 0
    int b=0,x,y,p;
    
    for (y = 0; y < H; ++y){
        p = largest_product_in_series(GRID + y*W, W, 4);
        if (b<p) b=p;
    }
    
    for (x = 0; x < W; ++x){
        p = largest_product_in_series(GRID + x, W, 4, W);
        if (b<p) b=p;
    }
    
    for (x = 0; x < W - 3; ++x){
        p = largest_product_in_series(GRID + x, 20 - x, 4, W+1);
        if (b<p) b=p;
    }
    
    for (y = 1; y < H - 3; ++y){
        p = largest_product_in_series(GRID + y*W, 20 - y, 4, W+1);
        if (b<p) b=p;
    }
    
    for (x = 0; x < W - 3; ++x){
        p = largest_product_in_series(GRID + x, 4 + x, 4, W-1);
        if (b<p) b=p;
    }
    
    for (y = 1; y < H - 3; ++y){
        p = largest_product_in_series(GRID + y*W + 16, 20 - y, 4, W-1);
        if (b<p) b=p;
    }
#endif
    
    int b=0,x,y,i,p;
    
    for (y = 0; y < H; ++y){
        for (x = 0; x < W - 3; ++x){
            p = 1;
            for (i = 0; i < 4; ++i){
                p *= GRID[GET_I(x+i,y)];
            }
            if (p>b) b=p;
        }
    }
    
    for (y = 0; y < H - 3; ++y){
        for (x = 0; x < W; ++x){
            p = 1;
            for (i = 0; i < 4; ++i){
                p *= GRID[GET_I(x,y+i)];
            }
            if (p>b) b=p;
        }
    }
    
    for (y = 0; y < H - 3; ++y){
        for (x = 0; x < W - 3; ++x){
            p = 1;
            for (i = 0; i < 4; ++i){
                p *= GRID[GET_I(x+i,y+i)];
            }
            if (p>b) b=p;
        }
    }
    
    for (y = 0; y < H - 3; ++y){
        for (x = 0; x < W - 3; ++x){
            p = 1;
            for (i = 0; i < 4; ++i){
                p *= GRID[GET_I(x+3-i,y+i)];
            }
            if (p>b) b=p;
        }
    }
    
    Euler_Result result;
    result.product = b;
    return result;
}

void euler_print(Euler_Result answer, Euler_In in){
    printf("%d\n", answer.product);
}

#define EULER_CHECK

bool euler_check(Euler_Result answer, Euler_In in){
    bool result = 1;
    
    int b=0,x,y,i,p;
    
    for (y = 0; y < H; ++y){
        for (x = 0; x < W - 3; ++x){
            p = 1;
            for (i = 0; i < 4; ++i){
                p *= GRID[GET_I(x+i,y)];
            }
            if (p>b) b=p;
        }
    }
    
    for (y = 0; y < H - 3; ++y){
        for (x = 0; x < W; ++x){
            p = 1;
            for (i = 0; i < 4; ++i){
                p *= GRID[GET_I(x,y+i)];
            }
            if (p>b) b=p;
        }
    }
    
    for (y = 0; y < H - 3; ++y){
        for (x = 0; x < W - 3; ++x){
            p = 1;
            for (i = 0; i < 4; ++i){
                p *= GRID[GET_I(x+i,y+i)];
            }
            if (p>b) b=p;
        }
    }
    
    for (y = 0; y < H - 3; ++y){
        for (x = 0; x < W - 3; ++x){
            p = 1;
            for (i = 0; i < 4; ++i){
                p *= GRID[GET_I(x+3-i,y+i)];
            }
            if (p>b) b=p;
        }
    }
    
    printf("answer = %d\n", b);
    result = (b == answer.product);
    
    return result;
}
#endif
