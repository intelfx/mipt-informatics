#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/*
 * Lexer
 */

struct {
    union {
        long value;
        char op;
    };

    enum {
        TYPE_NONE,
        TYPE_VALUE,
        TYPE_OP,
        TYPE_PAREN
    } type;
} lexem;

void lex_init()
{
    lexem.type = TYPE_NONE;
}

void lexic_error()
{
    fprintf (stderr, "invalid character in input\n");
    abort();
}

void lex_fetch()
{
    int c;

    if (lexem.type != TYPE_NONE) {
        return;
    }

    do {
        c = getchar();
    } while (isspace (c));

    if (c != EOF) {
        if (isdigit (c)) {
            ungetc (c, stdin);
            lexem.type = TYPE_VALUE;
            scanf ("%ld", &lexem.value);
        } else {
            switch (c) {
            case '+':
            case '-':
            case '*':
            case '/':
                lexem.type = TYPE_OP;
                break;

            case '(':
            case ')':
                lexem.type = TYPE_PAREN;
                break;

            default:
                lexic_error();
            }

            lexem.op = c;
        }
    }
}

int lex_check (int type) {
    lex_fetch();
    return (lexem.type == type);
}

int lex_ok() {
    lex_fetch();
    return (lexem.type != TYPE_NONE);
}

void lex_consume() {
    lex_fetch();
    lexem.type = TYPE_NONE;
}

void syntax_error (const char* reason) {
    fprintf (stderr, "syntax error: %s (invalid lexem: ", reason);
    switch (lexem.type) {
    case TYPE_NONE:
        fprintf (stderr, "<none>");
        break;

    case TYPE_OP:
        fprintf (stderr, "op '%c'", lexem.op);
        break;

    case TYPE_PAREN:
        fprintf (stderr, "parenthesis '%c'", lexem.op);
        break;

    case TYPE_VALUE:
        fprintf (stderr, "value '%ld'", lexem.value);
        break;

    default:
        fprintf (stderr, "<INVALID>");
        break;
    }
    fprintf (stderr, ")\n");
    abort();
}

void syntax_check (int r, const char* reason) {
    if (!r) {
        syntax_error (reason);
    }
}

long parse_expression();

long parse_value_or_parentheses (int or_0)
{
    long value = 0;

    if (lex_check (TYPE_VALUE)) {
        value = lexem.value;
        lex_consume();
    } else if (lex_check (TYPE_PAREN) && lexem.op == '(') {
        lex_consume();
        value = parse_expression();
        syntax_check (lex_check (TYPE_PAREN) && lexem.op == ')', "expected closing parenthesis");
        lex_consume();
    } else if (!or_0) {
        syntax_error ("expected value or opening parenthesis");
    }

    return value;
}

long parse_expression()
{
    long result = parse_value_or_parentheses (1);

    while (lex_ok() && lex_check (TYPE_OP)) {
        lex_consume();
        switch (lexem.op) {
        case '+':
            result += parse_value_or_parentheses (0);
            break;

        case '-':
            result -= parse_value_or_parentheses (0);
            break;

        case '*':
            result *= parse_value_or_parentheses (0);
            break;

        case '/':
            result /= parse_value_or_parentheses (0);
            break;

        default:
            syntax_error ("wrong operator");
        }
    }

    return result;
}

int main()
{
    long value = parse_expression();
    if (lex_ok()) {
        syntax_error ("expected end of input");
    }
    printf ("%ld\n", value);
}
