#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


#define sxy(sudoku, x, y) (sudoku[(x)/3][(y)/3][(x)%3][(y)%3])
#define cxy(cell_candidate, x, y, n) (cell_candidate[(x)/3][(y)/3][(x)%3][(y)%3][(n)])

typedef struct
{
    int block_candidate[3][3][10];
    int cell_candidate[3][3][3][3][10];
    int sudoku[3][3][3][3];
}
Sudoku;

/*---------------------------------------------------------------------------*/
/* clear.c                                                                   */
/*---------------------------------------------------------------------------*/
int clear(Sudoku *sudoku);
int clear_block_double_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...);
int clear_block_alligned_block(Sudoku *sudoku, int xb, int yb, ...);
int clear_candidate(Sudoku *sudoku, int xb, int yb, int xc, int yc, int c);
int clear_filled_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...);
int clear_line_double_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...);
int clear_unique_block_double_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...);

/*---------------------------------------------------------------------------*/
/* s.c                                                                       */
/*---------------------------------------------------------------------------*/
int all_blocks(Sudoku *sudoku, int (*block_function)(Sudoku *sudoku, int xb, int yb, ...), ...);
int all_cells(Sudoku *sudoku, int (*cell_function)(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...), ...);
int fill(Sudoku *sudoku);
int init(Sudoku *sudoku);
int init_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...);
int print(Sudoku *sudoku);
int print_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...);
int resolve(Sudoku *sudoku);

/*---------------------------------------------------------------------------*/
/* solve.c                                                                   */
/*---------------------------------------------------------------------------*/
int solve(Sudoku *sudoku);
int solve_block(Sudoku *sudoku, int xb, int yb, ...);  

int clear(Sudoku *sudoku)
{
    #ifdef __TRACE__
        printf("clear()\n");
        printf("clear_filled()\n");
    #endif 
    if(all_cells(sudoku, clear_filled_cell))
        return 1;
    #ifdef __TRACE__
        printf("clear_block_alligned()\n");
    #endif 
    if(all_blocks(sudoku, clear_block_alligned_block))
        return 1;
    #ifdef __TRACE__
        printf("clear_block_double()\n");
    #endif 
    #ifdef __TRACE__
        printf("clear_unique_block_double()\n");
    #endif 
    if(all_cells(sudoku, clear_unique_block_double_cell))
        return 1;
    if(all_cells(sudoku, clear_block_double_cell))
        return 1;
    #ifdef __TRACE__
        printf("clear_line_double()\n");
    #endif 
    if(all_cells(sudoku, clear_line_double_cell))
        return 1;
    return 0;
}

int clear_block_alligned_block(Sudoku *sudoku, int xb, int yb, ...)
{
    int rc;
    int xl;
    int yl;

    for(int n = 1; n < 10; n++)
    {
        if(sudoku -> block_candidate[xb][yb][n] < 2 || sudoku -> block_candidate[xb][yb][n] > 3)
            continue;
        xl = -1;
        yl = -1;
        for(int xc = 0; xc < 3; xc++)
        {
            for(int yc = 0; yc < 3; yc++)
            {
                if(sudoku -> cell_candidate[xb][yb][xc][yc][n] == n)
                {
                    if(xl == -1)
                        xl = xc;
                    else
                    {
                        if(xl != xc)
                            xl = -2;
                    }
                    if(yl == -1)
                        yl = yc;
                    else
                    {
                        if(yl != yc)
                            yl = -2;
                    }
                }
            }
        }
        if(xl >= 0)
        {
            printf("  found %d%d: x%d: %d\n", xb, yb, xl, n);
            for(int i = 0; i < 9; i++)
            {
                if(yb == i / 3)
                    continue;
                if(clear_candidate(sudoku, xb, i / 3, xl, i % 3, n))
                    rc = 1;
            }
        if(yl >= 0)
        {
            printf("  found %d%d: y%d: %d\n", xb, yb, yl, n);
            for(int i = 0; i < 9; i++)
            {
                if(xb == i / 3)
                    continue;
                if(clear_candidate(sudoku, i / 3, yb, i % 3, yl, n))
                    rc = 1;
            }
        }
            
        }
    }
    return rc;
}

int clear_block_double_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...)
{
    int c1;
    int c2;
    int n;
    int rc;

    if(sudoku -> cell_candidate[xb][yb][xc][yc][0] != 2)
        return 0;
    rc = 0;
    for(int x = xc; x < 3; x++)
    {
        for(int y = yc; y < 3; y++)
        {
            if(sudoku -> cell_candidate[xb][yb][x][y][0] != 2)
                continue;
            if(xc == x && yc == y)
                continue;
            c1 = 0;
            c2 = 0;
            for(n = 1; n < 10; n++)
            {
                if(sudoku -> cell_candidate[xb][yb][xc][yc][n] != sudoku -> cell_candidate[xb][yb][x][y][n])
                    break;
                if(sudoku -> cell_candidate[xb][yb][xc][yc][n])
                {
                    if(c1 == 0)
                        c1 = sudoku -> cell_candidate[xb][yb][xc][yc][n];
                    else
                        c2 = sudoku-> cell_candidate[xb][yb][xc][yc][n];
                }
            }
            if(n == 10)
            {
                #if __TRACE__
                    printf("  found %d%d%d%d-%d%d%d%d %d-%d\n", xb, yb, xc, yc, xb, yb, x, y, c1, c2);
                #endif
                for(int _x = 0; _x < 3; _x++)
                {
                    for(int _y = 0; _y < 3; _y++)
                    {
                        if(_x == xc && _y == yc)
                            continue;
                        if(_x == x && _y == y)
                            continue;
                        if(clear_candidate(sudoku, xb, yb, _x, _y, c1))
                            rc = 1;
                        if(clear_candidate(sudoku, xb, yb, _x, _y, c2))
                            rc = 1;
                    }
                }
                if(xc == x)
                {
                    for(int i = 0; i < 9; i++)
                    {
                        if(yb == i / 3)
                        {
                            if(yc == i % 3 || y == i % 3)
                                continue;
                        }
                        if(clear_candidate(sudoku, xb, i / 3, xc, i % 3, c1))
                            rc = 1;
                        if(clear_candidate(sudoku, xb, i / 3, xc, i % 3, c2))
                            rc = 1;
                    }
                }
                else if(yc == y)
                {
                    for(int i = 0; i < 9; i++)
                    {
                        if(xb == i / 3)
                        {
                            if(xc == i % 3 || x == i % 3)
                                continue;
                        }
                        if(clear_candidate(sudoku, i / 3, yb, i % 3, yc, c1))
                            rc = 1;
                        if(clear_candidate(sudoku, i / 3, yb, i % 3, yc, c2))
                            rc = 1;
                    }
                }
            }
        }
    }
    return rc;
}

int clear_candidate(Sudoku *sudoku, int xb, int yb, int xc, int yc, int c)
{
    if(sudoku -> cell_candidate[xb][yb][xc][yc][c])
    {
        #ifdef __TRACE__
            printf("  clear %d%d%d%d: %d\n", xb, yb, xc, yc, c);
        #endif
        sudoku -> block_candidate[xb][yb][c]--;
        sudoku -> cell_candidate[xb][yb][xc][yc][c] = 0;
        sudoku -> cell_candidate[xb][yb][xc][yc][0]--;
        return 1;
    }
    return 0;
}

int clear_filled_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...)
{
    int ch;
    int rc;

    ch = sudoku -> sudoku[xb][yb][xc][yc];
    if(ch == 0)
      return 0;
    rc = 0;
    for(int n = 1; n < 10; n++)
    {
        if(clear_candidate(sudoku, xb, yb, xc, yc, n))
            rc = 1;
    }
    for(int x = 0; x < 3; x++)
    {
        for(int y = 0; y < 3; y++)
        {
            if(clear_candidate(sudoku, xb, yb, x, y, ch))
                rc = 1;
        }
    }
    for(int i = 0; i < 9; i++)
    {
        if(clear_candidate(sudoku, xb, i / 3, xc, i % 3, ch))
            rc = 1;
        if(clear_candidate(sudoku, i / 3, yb, i % 3, yc, ch))
            rc = 1;
    }
    return rc;
}

int clear_line_double_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...)
{

    int c1;
    int c2;
    int n;
    int rc;

    if(sudoku -> cell_candidate[xb][yb][xc][yc][0] != 2)
        return 0;
    rc = 0;
    for(int i = xb * 3 + xc; i < 9; i++)
    {
        if(xb == i / 3)
            continue;
        c1 = 0;
        c2 = 0;
        for(n = 1; n < 10; n++)
        {
            if(sudoku -> cell_candidate[xb][yb][xc][yc][n] != sudoku -> cell_candidate[i / 3][yb][i % 3][yc][n])
                break;          
            if(sudoku -> cell_candidate[xb][yb][xc][yc][n])
            {
                if(c1 == 0)
                    c1 = sudoku -> cell_candidate[xb][yb][xc][yc][n];
                else
                    c2 = sudoku -> cell_candidate[xb][yb][xc][yc][n];
            }
        }
        if(n == 10)  
        {
            printf("  found x %d%d%d%d-%d%d%d%d %d-%d\n", xb, yb, xc, yc, i / 3, yb, i % 3, yc, c1, c2);
            for(int j = 0; j < 9; j++)
            {
                if(xb == j / 3 && xc == j % 3)
                    continue;
                if(i == j)
                    continue;
                if(clear_candidate(sudoku, j / 3, yb, j % 3, yc, c1))
                    rc = 1;
                if(clear_candidate(sudoku, j / 3, yb, j % 3, yc, c2))
                    rc = 1;
            }
        }
    }
    for(int i = yb * 3 + yc; i < 9; i++)
    {
        if(yb == i / 3)
            continue;
        c1 = 0;
        c2 = 0;
        for(n = 1; n < 10; n++)
        {
            if(sudoku -> cell_candidate[xb][yb][xc][yc][n] != sudoku -> cell_candidate[xb][i / 3][xc][i % 3][n])
                break;          
            if(sudoku -> cell_candidate[xb][yb][xc][yc][n])
            {
                if(c1 == 0)
                    c1 = sudoku -> cell_candidate[xb][yb][xc][yc][n];
                else
                    c2 = sudoku -> cell_candidate[xb][yb][xc][yc][n];
            }
        }
        if(n == 10)  
        {
            printf("  found y %d%d%d%d-%d%d%d%d %d-%d\n", xb, yb, xc, yc, xb, i / 3, yb, i % 3, c1, c2);
            for(int j = 0; j < 9; j++)
            {
                if(yb == j / 3 && yc == j % 3)
                    continue;
                if(i == j)
                    continue;
                if(clear_candidate(sudoku, xb, j / 3, yc, j % 3, c1))
                    rc = 1;
                if(clear_candidate(sudoku, xb, j / 3, yc, j % 3, c2))
                    rc = 1;
            }
        }
    }
    return rc;
}

int clear_unique_block_double_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...)
{
    int c1;
    int c2;
    int rc;

    rc = 0;
    c1 = 0;
    c2 = 0;

    if(sudoku -> sudoku[xb][yb][xc][yc] != 0)
        return 0;
    for(int n = 1; n < 10; n++)
    {
        if(sudoku -> block_candidate[xb][yb][n] != 2)
            continue;
        if(sudoku -> cell_candidate[xb][yb][xc][yc][n] == 0)
            continue;
        if(c1 == 0)
            c1 = n;
        else
            c2 = n;
    }
    if(c1 && c2)
        printf("  -found %d%d%d%d: %d%d\n", xb, yb, xc, yc, c1, c2);
    return rc;
}


int solve(Sudoku *sudoku)
{
    #ifdef __TRACE__
        printf("solve()\n");
    #endif
    if(all_blocks(sudoku, solve_block))
        return 1;
    return 0;
}

int solve_block(Sudoku *sudoku, int xb, int yb, ...)
{
    int rc;

    rc = 0;
    for(int n = 1; n < 10; n++)
    {
        if(sudoku -> block_candidate[xb][yb][n] != 1)
            continue;
        for(int xc = 0; xc < 3; xc++)
        {
            for(int yc = 0; yc < 3; yc++)
            {
                if(sudoku -> cell_candidate[xb][yb][xc][yc][n])
                {
                    #ifdef __TRACE__
                        printf("  solve %d%d%d%d: %d\n", xb, yb, xc, yc, n);
                    #endif
                    sudoku -> sudoku[xb][yb][xc][yc] = n;
                    rc = 1;
                    xc = 3;
                    yc = 3;

                }
            }
        }
    }
    return rc;
}

int all_blocks(Sudoku *sudoku, int (*block_function)(Sudoku *sudoku, int xb, int yb, ...), ...)
{
    va_list args;
    int rc;

    va_start(args, block_function);
    rc = 0;
    for(int xb = 0; xb < 3; xb++)
    {
        for(int yb = 0; yb < 3; yb++)
        {
            if(block_function(sudoku, xb, yb, args))
                rc = 1;
        }
    }
    va_end(args);
    return rc;
}

int all_cells(Sudoku *sudoku, int (*cell_function)(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...), ...)
{
    va_list args;
    int rc;

    va_start(args, cell_function);
    rc = 0;
    for(int xb = 0; xb < 3; xb++)
    {
        for(int yb = 0; yb < 3; yb++)
        {
            for(int xc = 0; xc < 3; xc++)
            {
                for(int yc = 0; yc < 3; yc++)
                {
                    if(cell_function(sudoku, xb, yb, xc, yc, args))
                        rc = 1;
                }
            }
        }
    }
    va_end(args);
    return rc;
}

int fill(Sudoku *sudoku)
{
    #ifdef __TRACE__
        printf("fill()\n");
    #endif
#if 1
    int s[9][9] = 
        {
            {0,0,0, 8,9,0, 0,0,0},
            {4,7,3, 2,0,0, 0,0,0},
            {0,1,0, 6,0,0, 0,0,0},

            {5,0,2, 0,7,0, 0,0,0},
            {0,8,0, 0,0,0, 4,9,0},
            {0,0,0, 0,0,8, 0,0,0},

            {0,0,0, 0,0,0, 0,0,3},
            {0,6,1, 4,0,9, 0,0,7},
            {0,0,8, 0,0,0, 0,0,2}
        };
#else 
    int s[9][9] = 
        {
            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},

            {1,2,3, 0,0,0, 1,2,3},
            {0,0,0, 0,5,6, 0,0,0},
            {0,8,9, 0,0,0, 0,8,9},

            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0},
            {0,0,0, 0,0,0, 0,0,0}
        };
#endif

    for(int x = 0; x < 9; x++)
    {
        for(int y = 0; y < 9; y++)
            sxy(sudoku -> sudoku, x, y) = s[y][x];
    }
    return 0;
}

int init(Sudoku *sudoku)
{
    #ifdef __TRACE__
        printf("init()\n");
    #endif
    return all_cells(sudoku, init_cell);
}

int init_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...)
{
    sudoku -> sudoku[xb][yb][xc][yc] = 0;
    for(int n = 1; n < 10; n++)
        sudoku -> cell_candidate[xb][yb][xc][yc][n] = n;
    sudoku -> cell_candidate[xb][yb][xc][yc][0] = 9;
    if(xc == 0 && yc ==0)
    {
        for(int n = 1; n < 10; n++)
                sudoku -> block_candidate[xb][yb][n] = 9;
    }
    return 0;
}

int main(void)
{
    Sudoku sudoku;

    #ifdef __TRACE__
        printf("main()\n");
    #endif
    init(&sudoku);
    print(&sudoku);
    fill(&sudoku);
    do
        print(&sudoku);
    while(resolve(&sudoku));
    return 0;
}

int print(Sudoku *sudoku)
{
    #ifdef __TRACE__
        printf("print()\n");
    #endif
    printf(" -----------------------\n");
    for(int y = 0; y < 9; y++)
    {
        if(y && y % 3 == 0)
            printf("|                       |\n");
        for(int x = 0; x < 9; x++)
        {
            if(x && x % 3 == 0)
                printf("  ");
            if(x == 0)
                printf("|");
            if(sxy(sudoku -> sudoku, x, y))
                printf(" %d", sxy(sudoku -> sudoku, x, y));
            else
                printf(" .");
            if(x == 8)
                printf(" |");
        }
        printf("\n");
    }
    printf(" -----------------------\n");
    #ifdef __TRACE__
        all_cells(sudoku, print_cell);
        printf("\n");
    #endif
    return 0;
}

int print_cell(Sudoku *sudoku, int xb, int yb, int xc, int yc, ...)
{
    if(xc == 0 && yc == 0)
    {
        printf("\n      ");
        for(int n = 1; n < 10; n++)
        {
            if(sudoku -> block_candidate[xb][yb][n])
                printf("%d", sudoku -> block_candidate[xb][yb][n]);
            else
                printf(".");
        }
        printf("\n");
    }    
    printf("%d%d%d%d: ", xb, yb, xc, yc);
    for(int n = 1; n < 10; n++)
    {
        if(sudoku -> cell_candidate[xb][yb][xc][yc][n])
            printf("%d", sudoku -> cell_candidate[xb][yb][xc][yc][n]);
        else
            printf(".");
    }
    if(sudoku -> cell_candidate[xb][yb][xc][yc][0])
        printf("(%d)\n", sudoku -> cell_candidate[xb][yb][xc][yc][0]);
    else
        printf("(0)(%d)\n", sudoku -> sudoku[xb][yb][xc][yc]);
    return 0;
}

int resolve(Sudoku *sudoku)
{
    #ifdef __TRACE__
        printf("resolve()\n");
    #endif 
    if(clear(sudoku))
        return 1;
    if(solve(sudoku))
        return 1;
    return 0;
}
