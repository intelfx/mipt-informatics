#include <stdio.h>
#include <stdlib.h>
#include <string.h>




struct Stack
{
    int n;
    int size;
    int * a;
};

struct Stack * stack_create(int size)
{
    struct Stack* st = (struct Stack*)malloc(sizeof(struct Stack));
    st->n = 0;
    st->a = (int*)malloc(sizeof(int) * 100000);
    return st;
};

void stack_push(struct Stack * s, int x)
{
    (s->a)[s->n] = x;
    s->n++;
}

int stack_pop(struct Stack * s)
{
    int mem = (s->a)[s->n - 1];
    s->n--;
    return mem;
}
int stack_get(struct Stack * s)
{
    return (s->a)[s->n - 1];
}
void stack_print(struct Stack * s)
{
    if(s->n == 0)
    {
        printf("Empty stack\n");
        return;
    }
    int i = 0;
    for(i = 0; i < s->n; i++)
        printf("%d ", (s->a)[i]);
    printf("\n");
}
int  stack_size(struct Stack * s)
{
    return s->n;
}
int stack_is_empty(struct Stack * s)
{
    return (s->n == 0) ? 1 : 0;
}
void stack_clear(struct Stack * s)
{
    s->n = 0;
}
void stack_destroy(struct Stack * s)
{
    free(s->a);
    free(s);
}

long long int atoi_nn(char* str)
{
    long long int res = 0;
    int st = 1;
    int i = 0;
    for(i = strlen(str) - 1; i >= 0; i--)
    {
        res += (str[i] - '0') * st;
        st *= 10;
    }

    return res;
}

int inside(char a)
{
    int i = 0;
    for(i = '0'; i <= '9'; i++) if(a == i) return 1;

    return 0;
}

int main()
{
    struct Stack* stk = stack_create(12);

    char* str = (char*)malloc(1028 * sizeof(char));

    scanf("%s", str);
    str = strcat(str, ")");
    char* str2 = (char*)malloc(3008 * sizeof(char));
    str2[0] = '(';
    str2[1] = '\0';
    long long int len = 1;
    long long int len_str = strlen(str);
    long long int i = 0;
    for(i = 0; i <= len_str; i++)
    {
        if(str[i] != ' ' && str[i] != 'q')
        {
            if(str[i] == '-')
            {
                if(inside(str2[len - 1]))
                {
                    len++;
                    str2 = strcat(str2,"-");
                }
                else if(str2[len - 1] == '+') str2[len - 1] = '-';
                else if(str2[len - 1] == '-') str2[len - 1] = '+';
                else if(str2[len - 1] == '*')
                {
                    str2[len - 1] = '(';
                    str2 = strcat(str2, "0-1)*");
                    len += 6;
                }
                else if(str2[len - 1] == '(')
                {
                    str2 = strcat(str2, "0-");
                    len += 2;
                }
                else if(str2[len - 1] == ')')
                {
                    str2 = strcat(str2, "-");
                    len ++;
                }
            }
            else if(str[i] == '+')
            {
                if(inside(str2[len - 1]))
                {
                    len++;
                    str2 = strcat(str2, "+");
                }
                else if(str2[len - 1] == '+') str2[len - 1] = '+';
                else if(str2[len - 1] == '-') str2[len - 1] = '-';
                else if(str2[len - 1] == '*') str2[len - 1] = '*';
                else if(str2[len - 1] == '(')
                {
                    len += 2;
                    str2 = strcat(str2, "0+");
                }
                else if(str2[len - 1] == ')')
                {
                    len++;
                    str2 = strcat(str2, "+");
                }
            }
            else if(str[i] == '*')
            {
                if(inside(str2[len - 1]))
                {
                    len++;
                    str2 = strcat(str2, "*");
                }
                else if(str2[len - 1] == '+') str2[len - 1] = '*';
                else if(str2[len - 1] == '-')
                {
                    str2[len - 1] = '*';
                    str2 = strcat(str2, "(0-1)*");
                    len += 6;
                }
                else if(str2[len - 1] == '*') str2[len - 1] = '*';
                else if(str2[len - 1] == '(') str2[len - 1] = '(';
                else if(str2[len - 1] == ')')
                {
                    str2 = strcat(str2, "*");
                    len++;
                }

            }
            else if(str[i] == '(')
            {
                if(str2[len - 1] == '(')
                {
                    len += 2;
                    str2 = strcat(str2,"0+");
                }
                str2[len] = str[i];
                str2[len + 1]='\0';
                len++;
            }
            else if(str[i] == ')')
            {
                if(str2[len - 1] == '(')
                {
                    str2 = strcat(str2, "0");
                    len++;
                }
                if(str2[len - 1] == '+')
                {
                    str2 = strcat(str2, "0");
                    len++;
                }
                if(str2[len - 1] == '*')
                {
                    str2 = strcat(str2, "1");
                    len++;
                }
                str2[len] = str[i];
                str2[len + 1] = 0;
                len++;
            }
            else if(inside(str[i]))
            {
                str2[len] = str[i];
                str2[len + 1]='\0';
                len++;
            }
            else if(str[i] == 0) str2[len] = 0;
            else
            {
                stack_destroy(stk);
                free(str);
                free(str2);
                exit(0);
            }
        }
    }
    struct Stack* stk_1 = stack_create(123);
    int fl = 0;
    for(i = 0; i < len - 1; i++)
    {
        if(str2[i] != ' ' && str2[i] != 'q')
        {
            if(str2[i] == '(')
                fl = 1;
            else if(str2[i] == ')')
            {
                long long int y = stack_pop(stk);
                long long int x = stack_pop(stk);
                long long int dey = stack_pop(stk_1);
                if(dey == '+') stack_push(stk, x + y);
                else if(dey == '-') stack_push(stk, x - y);
                else if(dey == '*') stack_push(stk, x * y);
            }
            else if(str2[i] == '+') stack_push(stk_1, '+');
            else if(str2[i] == '-') stack_push(stk_1, '-');
            else if(str2[i] == '*') stack_push(stk_1, '*');
            else
            {
                if(fl == 1)
                {
                    fl = 0;
                    char tmpc[20];
                    int j = 0;
                    while(inside(str2[i]))
                    {
                        tmpc[j] = str2[i];
                        i++;
                        j++;
                    }
                    i--;
                    tmpc[j] = '\0';
                    long long int x = atoi_nn(tmpc);
                    stack_push(stk, x);

                }
                else
                {
                    long long int dey = stack_pop(stk_1);
                    long long int x = stack_pop(stk);
                    char tmpc[20];
                    int j = 0;
                    while(inside(str2[i]))
                    {
                        tmpc[j] = str2[i];
                        i++;
                        j++;
                    }
                    i--;
                    tmpc[j] = '\0';
                    long long int y = atoi_nn(tmpc);
                    if(dey == '+') stack_push(stk, x + y);
                    else if(dey == '-') stack_push(stk, x - y);
                    else if(dey == '*') stack_push(stk, x * y);
                }
            }
        }
    }

    for(; stack_size(stk_1) ;)
    {
        long long int y = stack_pop(stk);
        long long int x = stack_pop(stk);
        long long int dey = stack_pop(stk_1);
        if(dey == '+') stack_push(stk, x + y);
        else if(dey == '-') stack_push(stk, x - y);
        else if(dey == '*') stack_push(stk, x * y);
    }
    printf("%d\n", stack_pop(stk));
    stack_destroy(stk);
    stack_destroy(stk_1);
    free(str);
    free(str2);
    return 0;
}