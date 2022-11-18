/*
 * YOUR INFO HERE!
 */


int combo(int n, int r){
    int data[2000];
    int rl;
    int i,j;
    
    if (r == 0) return 1;
    if (r == n) return 1;
    rl = n - r;
    if (r == 1) return n;
    if (rl == 1) return n;
    if (rl < r) r = rl; 
    r -= 2;
    rl = n - r;

    if (r == 0){
in0:
    if (i == rl) goto out0;
    j = j + i;
    i = i + 1;
    goto in0;
out0:
    return j;
    }
    
    i = 4;
    j = 6;
in1:
    data[i] = j;
    if (i == rl) goto out1;
    j = j + i;
    i = i + 1;
    goto in1;
out1:

}

