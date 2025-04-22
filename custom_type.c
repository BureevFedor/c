#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const unsigned int base = 1000000000; // 10^9
const unsigned int base_digits = 9;
const unsigned int max_cnt = 35; // 309/9

typedef struct {
    unsigned int numbers[35];
} uint1024_t;

void init(uint1024_t *x)
{
    memset(x->numbers, 0, sizeof(x->numbers));
}

// count numbers
int get_cnt(uint1024_t* x)
{
    int cnt = 1; // one digit for zero value if empty
    for (int i = 0; i < max_cnt; ++i)
    {
        if (x->numbers[i] > 0)
            cnt = i+1;
    }
    return cnt;
}

void printf_value(uint1024_t x)
{
    int cnt = get_cnt(&x);

    printf("%u", x.numbers[cnt-1]); // don't print starting zeros
    for (int i = cnt-2; i >= 0; i--)
        printf("%09u", x.numbers[i]);
}

uint1024_t from_uint(unsigned int x)
{
    uint1024_t res;

    init(&res);
    res.numbers[0] = x % base;
    res.numbers[1] = x / base;
    return res;
}

uint1024_t add_op(uint1024_t x, uint1024_t y)
{
    uint1024_t res;
    unsigned int perenos = 0;

    init(&res);
    for (int i = 0; i < max_cnt; ++i)
    {
        res.numbers[i] = x.numbers[i] + y.numbers[i] + perenos;
        perenos = 0;
        if (res.numbers[i] >= base)
        {
            res.numbers[i] = res.numbers[i] - base;
            perenos = 1;
        }
    }
    return res;
}

// 1 - x > y, 2 - x < y, 3 - x = y
int cmp(uint1024_t x, uint1024_t y)
{
    for (int i = max_cnt-1; i >=0; i--)
    {
        if (x.numbers[i] > y.numbers[i])
            return 1;
        if (y.numbers[i] > x.numbers[i])
            return 2;
    }
    return 3;
}

// it is expected that x > y
uint1024_t subtr_op(uint1024_t x, uint1024_t y)
{
    uint1024_t res;
    unsigned int zaem = 0;

    init(&res);

    int res_cmp = cmp(x,y);
    if ((res_cmp == 2) || (res_cmp == 3))
        return res;

    for (int i = 0; i < max_cnt; ++i)
    {
        if (x.numbers[i] >= (y.numbers[i] + zaem))
        {
            res.numbers[i] = x.numbers[i] - y.numbers[i] - zaem;
            zaem = 0;
        }
        else
        {
            res.numbers[i] = base - (y.numbers[i] + zaem - x.numbers[i]);
            zaem = 1;
        }
    }
    return res;
}

uint1024_t mult_uint(uint1024_t x, int cnt, unsigned int y)
{
    uint1024_t res;
    unsigned long long value;
    unsigned long long perenos = 0;

    init(&res);
    for (int i = 0; i < cnt; i++) {
        value = (unsigned long long)x.numbers[i] * (unsigned long long)y + perenos;
        if (value >= (unsigned long long)base)
        {
            res.numbers[i] = (unsigned int)(value % (unsigned long long)base);
            perenos = value / (unsigned long long)base;
        }
        else
        {
            res.numbers[i] = (unsigned int)value;
            perenos = 0;
        }
    }
    if (perenos != 0)
        res.numbers[cnt] = (unsigned int)perenos;

    return res;
}

void shift_left(uint1024_t* x, int pos)
{
    for (int i = max_cnt-1; i >= 0; i--)
    {
        if (i < pos)
            x->numbers[i] = 0;
        else
            x->numbers[i] = x->numbers[i-pos];
    }
}

uint1024_t mult_op(uint1024_t x, uint1024_t y)
{
    uint1024_t res;

    init(&res);
    int x_cnt = get_cnt(&x);
    int y_cnt = get_cnt(&y);
    for (int i = 0; i < y_cnt; i++)
    {
        if (y.numbers[i] != 0)
        {
            uint1024_t z = mult_uint(x, x_cnt, y.numbers[i]);
            shift_left(&z, i);
            res = add_op(res, z);
        }
    }
    return res;
}

void scanf_value(uint1024_t *x)
{
    char str[310];
    int n;
    int number;
    int cnt;
    int digit;
    int numbers_cnt = 0;

    init(x);
    scanf("%309s", str); // read x in string format
    n = strlen(str); // number of decimal digits
    while (n > 0)
    {
        cnt = 9; // base digits
        if (n < cnt)
            cnt = n;
        number = 0;
        for (int i = n-cnt; i < n; i++)
        {
            digit = str[i] - '0'; // create decimal digit from char
            number = number*10 + digit;
        }
        x->numbers[numbers_cnt] = number;
        numbers_cnt++;
        n = n-cnt;
    }
}

int main(int argc, char *argv[])
{
    unsigned int n;
    uint1024_t x;
    uint1024_t y;

    printf("Enter n: ");
    scanf("%u", &n);

    printf("Enter x: ");
    scanf_value(&x);

    printf("Enter y: ");
    scanf_value(&y);

    printf("\n");

    printf("from_uint(n):\n");
    uint1024_t a = from_uint(n);
    printf_value(a);
    printf("\n");

    printf("x + y =\n");
    uint1024_t b = add_op(x, y);
    printf_value(b);
    printf("\n");

    printf("x - y =\n");
    uint1024_t c = subtr_op(x, y);
    printf_value(c);
    printf("\n");

    printf("x * y =\n");
    uint1024_t d = mult_op(x, y);
    printf_value(d);
    printf("\n");

    /*printf("mult_uint(x,n):\n");
    uint1024_t e = mult_uint(x, get_cnt(&x), n);
    printf_value(e);
    printf("\n");*/

    return 0;
}