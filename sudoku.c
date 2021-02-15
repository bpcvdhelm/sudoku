/*----------------------------------------------------------------------------*/
/* sudoku.c (c) 2021 Bernard van der Helm, The Hague, The Netherlands         */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

#ifdef __TRACE__
#define Trace(...) printf(__VA_ARGS__)
#else
#define Trace(...)
#endif

#define AllPremiseBitsOn 0x1ff
#define X 0
#define Y 1


/*----------------------------------------------------------------------------*/
/* cleared       : number of premises cleared so far                          */
/* premise_bit   : conversion from bit position to number, 0->1...8->9        */
/* cell_premise  : premises in bit positions, 0x01->1, 0x10->2, ..., 0x100->9 */
/* cell_premises : number of premises in cell                                 */
/* block_premises: number of premises in block                                */
/* line_premises : number of premises in line x and y                         */
/*----------------------------------------------------------------------------*/

typedef struct
{
    int cleared;
    int premise_bit[9];
    
    int cell_premise[3][3][3][3];

    int cell_premises[3][3][3][3];
    int block_premises[3][3][9];
    int line_premises[2][9][9];
}
Sudoku;

/* common functions */
int all_blocks(Sudoku *s, int (*block_function)(Sudoku *s, int xb, int yb));
int all_cells(Sudoku *s, int (*cell_function)(Sudoku *s, int xb, int yb, int xc, int yc));
int number(int premise_bit);

/* core functions */
int deduce(Sudoku *s);
int fill(Sudoku *s);
int init(Sudoku *s);
int main(void);
int print(Sudoku *s);
int print_cell(Sudoku *s, int xb, int yb, int xc, int yc);

/* clear functions */
int clear_premise(Sudoku *s, int xb, int yb, int xc, int yc, int p, char *reason);
int verify(Sudoku *s);
int verify_block(Sudoku *s, int xb, int yb);
int verify_cell(Sudoku *s, int xb, int yb, int xc, int yc);
int verify_xline(Sudoku *s);
int verify_yline(Sudoku *s);

/* deduce block functions */
int deduce_block_exclusive_block(Sudoku *s, int xb, int yb);
int deduce_block_exclusive_group_cell(Sudoku *s, int xb, int yb, int xc, int yc);
int deduce_block_group_cell(Sudoku *s, int xb, int yb, int xc, int yc);
int deduce_block_solved_cell(Sudoku *s, int xb, int yb, int xc, int yc);

/* deduce line functions */
int deduce_xline_block_group_block(Sudoku *s, int xb, int yb);
int deduce_yline_block_group_block(Sudoku *s, int xb, int yb);
int deduce_xline_group_cell(Sudoku *s, int xb, int yb, int xc, int yc);
int deduce_yline_group_cell(Sudoku *s, int xb, int yb, int xc, int yc);
int deduce_xline_exclusive_group_cell(Sudoku *s, int xb, int yb, int xc, int yc);
int deduce_yline_exclusive_group_cell(Sudoku *s, int xb, int yb, int xc, int yc);

/* deduce xwing functions */
int deduce_xwing_x(Sudoku *s);
int deduce_xwing_y(Sudoku *s);

/* sample sudokus */
#define S1 \
{\
    {0,0,0, 8,9,0, 0,0,0},\
    {4,7,3, 2,0,0, 0,0,0},\
    {0,1,0, 6,0,0, 0,0,0},\
\
    {5,0,2, 0,7,0, 0,0,0},\
    {0,8,0, 0,0,0, 4,9,0},\
    {0,0,0, 0,0,8, 0,0,0},\
\
    {0,0,0, 0,0,0, 0,0,3},\
    {0,6,1, 4,0,9, 0,0,7},\
    {0,0,8, 0,0,0, 0,0,2}\
}

#define S2 \
{\
    {0,0,0, 8,2,0, 0,0,0},\
    {0,0,0, 7,0,0, 0,1,6},\
    {2,0,0, 4,0,0, 0,0,0},\
\
    {6,0,0, 5,0,0, 0,0,0},\
    {7,8,0, 6,9,0, 0,0,0},\
    {0,5,4, 0,0,0, 8,0,2},\
\
    {0,7,0, 0,0,0, 0,8,0},\
    {0,2,0, 0,7,6, 0,5,0},\
    {0,0,9, 0,0,0, 0,0,1}\
}

#define S3 \
{\
    {0,0,0, 7,8,5, 6,0,0},\
    {0,0,8, 0,0,0, 0,0,4},\
    {0,6,0, 0,0,0, 0,0,0},\
\
    {4,1,0, 0,0,8, 2,0,0},\
    {0,0,0, 0,0,0, 0,3,0},\
    {0,0,0, 0,0,2, 0,7,0},\
\
    {0,9,0, 3,6,1, 0,0,0},\
    {0,0,0, 0,0,0, 5,0,7},\
    {0,0,2, 9,0,0, 0,0,0}\
}

#define S4 \
{\
    {0,0,0, 4,0,0, 0,0,0},\
    {0,5,0, 7,0,0, 0,0,9},\
    {0,0,0, 9,0,8, 6,0,1},\
\
    {9,6,4, 0,0,0, 0,8,0},\
    {0,0,7, 0,0,0, 9,0,0},\
    {0,0,0, 0,0,0, 0,0,0},\
\
    {3,1,0, 0,0,2, 0,0,0},\
    {0,0,0, 8,0,0, 0,0,7},\
    {2,0,0, 0,0,0, 1,0,3}\
}

#define S5 \
{\
    {0,0,0, 0,0,0, 9,4,0},\
    {6,0,0, 0,0,0, 2,7,0},\
    {8,2,0, 0,4,9, 6,0,0},\
\
    {0,7,4, 0,0,0, 0,0,0},\
    {1,0,0, 7,6,0, 0,0,0},\
    {0,6,2, 0,0,5, 0,8,0},\
\
    {0,0,0, 0,5,7, 0,2,3},\
    {0,0,0, 0,0,0, 0,0,0},\
    {7,5,3, 2,0,4, 0,0,0}\
}

#define S6 \
{\
    {0,0,0, 0,0,0, 0,0,0},\
    {0,0,0, 0,0,0, 0,0,0},\
    {0,0,0, 0,0,0, 0,0,0},\
\
    {0,0,0, 0,0,0, 0,0,0},\
    {0,0,0, 0,0,0, 0,0,0},\
    {0,0,0, 0,0,0, 0,0,0},\
\
    {0,0,0, 0,0,0, 0,0,0},\
    {0,0,0, 0,0,0, 0,0,0},\
    {0,0,0, 0,0,0, 0,0,0}\
}

#define S7 \
{\
    {0,0,0, 0,0,0, 0,4,7},\
    {0,0,0, 0,0,3, 0,0,8},\
    {0,9,0, 0,0,6, 0,0,0},\
\
    {0,6,4, 0,8,0, 0,5,0},\
    {0,0,5, 0,0,0, 7,9,0},\
    {0,0,0, 0,6,2, 0,0,0},\
\
    {1,0,0, 8,0,0, 0,0,0},\
    {4,0,2, 1,0,0, 0,0,0},\
    {0,0,0, 0,0,0, 5,0,4}\
}

#define S \
{\
    {0,6,0, 0,9,0, 0,1,0},\
    {5,0,0, 7,0,8, 0,0,9},\
    {0,0,7, 0,0,0, 3,0,0},\
\
    {0,5,0, 0,6,0, 0,4,0},\
    {4,0,0, 8,0,7, 0,0,5},\
    {0,1,0, 0,2,0, 0,7,0},\
\
    {0,0,5, 0,0,0, 7,0,0},\
    {2,0,0, 1,0,3, 0,0,4},\
    {0,9,0, 0,7,0, 0,2,0}\
}

/*----------------------------------------------------------------------------*/
/* all_blocks                                                                 */
/*----------------------------------------------------------------------------*/
int all_blocks(Sudoku *s, int (*block_function)(Sudoku *s, int xb, int yb))
{
    int rc;

    rc = 0;

    /* execute function on all blocks */
    for(int xb = 0; xb < 3; xb++)
    {
        for(int yb = 0; yb < 3; yb++)
        {
            if(block_function(s, xb, yb))
                rc = 1;
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* all_cells                                                                  */
/*----------------------------------------------------------------------------*/
int all_cells(Sudoku *s, int (*cell_function)(Sudoku *s, int xb, int yb, int xc, int yc))
{
    int rc;

    rc = 0;

    /* execute function on all cells */
    for(int xb = 0; xb < 3; xb++)
    {
        for(int yb = 0; yb < 3; yb++)
        {
            for(int xc = 0; xc < 3; xc++)
            {
                for(int yc = 0; yc < 3; yc++)
                {
                    if(cell_function(s, xb, yb, xc, yc))
                        rc = 1;
                }
            }
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* clear_premise                                                              */
/*----------------------------------------------------------------------------*/
int clear_premise(Sudoku *s, int xb, int yb, int xc, int yc, int p, char *reason)
{
    if(s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
    {
        /* clear premise */
        Trace("    clear (%3d/648) c%d%d%d%d p%d %s\n", s -> cleared + 1, xb, yb, xc, yc, p + 1, reason);
        
        s -> cleared++;
        
        s -> cell_premise[xb][yb][xc][yc] &= ~s -> premise_bit[p];

        s -> cell_premises[xb][yb][xc][yc]--;
        s -> block_premises[xb][yb][p]--;
        s -> line_premises[X][yb * 3 + yc][p]--;
        s -> line_premises[Y][xb * 3 + xc][p]--;

        #ifdef __TRACE__
            /* check integrity */
            verify(s);
        #endif

        return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* deduce                                                                     */
/*----------------------------------------------------------------------------*/
int deduce(Sudoku *s)
{
    Trace("deduce()\n");

    /* deduce blocks */
    Trace("deduce_block_solved()\n");
    if(all_cells(s, deduce_block_solved_cell))
        return 1;
    Trace("deduce_block_groups()\n");
    if(all_cells(s, deduce_block_group_cell))
        return 1;
    Trace("deduce_block_exclusives()\n");
    if(all_blocks(s, deduce_block_exclusive_block))
        return 1;
    Trace("deduce_block_exclusive_groups()\n");
    if(all_cells(s, deduce_block_exclusive_group_cell))
        return 1;

    /* deduce lines */
    Trace("deduce_xline_groups()\n");
    if(all_cells(s, deduce_xline_group_cell))
        return 1;
    Trace("deduce_yline_groups()\n");
    if(all_cells(s, deduce_yline_group_cell))
        return 1;
    Trace("deduce_xline_block_groups()\n");
    if(all_blocks(s, deduce_xline_block_group_block))
        return 1;
    Trace("deduce_yline_block_groups()\n");
    if(all_blocks(s, deduce_yline_block_group_block))
        return 1;
    Trace("deduce_xline_exclusive_groups()\n");
    if(all_cells(s, deduce_xline_exclusive_group_cell))
        return 1;
    Trace("deduce_yline_exclusive_groups()\n");
    if(all_cells(s, deduce_yline_exclusive_group_cell))
        return 1;   

    /* deduce x-wings */
    if(deduce_xwing_x(s))
        return 1;
    if(deduce_xwing_y(s))
        return 1;

    return 0;
}

/*----------------------------------------------------------------------------*/
/* deduce_block_exclusive_block                                               */
/*----------------------------------------------------------------------------*/
int deduce_block_exclusive_block(Sudoku *s, int xb, int yb)
{
    int rc;

    rc = 0;

    /* find premise unique in block */
    for(int p = 0; p < 9; p++)
    {
        /* check if this is a block exclusive */
        if(s -> block_premises[xb][yb][p] == 1)
        {
            Trace("  found b%d%d p%d\n", xb, yb, p + 1);

            /* we found an excusive cell within the block */
            for(int xc = 0; xc < 3; xc++)
            {
                for(int yc = 0; yc < 3; yc++)
                {
                    /* make it also a cell exclusive */
                    if(s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
                    {
                        for(int p2 = 0; p2 < 9; p2++)
                        {
                            if(p2 != p)
                            {
                                if(clear_premise(s, xb, yb, xc, yc, p2, "deduce_block_exclusive_block()"))
                                    rc = 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_block_solved_cell                                                   */
/*----------------------------------------------------------------------------*/
int deduce_block_solved_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    int p;
    int rc;

    /* return if not deduced */
    if(s -> cell_premises[xb][yb][xc][yc] != 1)
        return 0;

    /* clear deduced number in other block cells */
    p = number(s -> cell_premise[xb][yb][xc][yc]);
    rc = 0;

    Trace("  found c%d%d%d%d p%d\n", xb, yb, xc, yc, p + 1);

    /* clear other cells in block */
    for(int x = 0; x < 3; x++)
    {
        for(int y = 0; y < 3; y++)
        {
            /* not yourself! */
            if(x == xc && y == yc)
                continue;
            if(clear_premise(s, xb, yb, x, y, p, "deduce_block_solved_cell()"))
                rc = 1;
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_xline_block_group_block                                             */
/*----------------------------------------------------------------------------*/
int deduce_xline_block_group_block(Sudoku *s, int xb, int yb)
{
    int l;
    int rc;
    int yc;

    rc = 0;

    /* find unique premises within the block x line  */
    for(int p = 0; p < 9; p++)
    {
        l = -1;
        for(yc = 0; yc < 3; yc++)
        {
            for(int xc = 0; xc < 3; xc++)
            {
                if(s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
                {
                    if(l == -1)
                    {
                        l = yc;
                        break;
                    }
                    else
                    {
                        l = -2;
                        break;
                    }
                }
            }
            if(l == -2)
                break;
        }
        if(l < 0)
            continue;

        Trace("  found b%d%d p%d y%d\n", xb, yb, p + 1, l);

        /* found double clear these in other cells */
        for(int i = 0; i < 9; i++)
        {
            if(xb == i / 3)
                continue;
            
            if(clear_premise(s, i / 3, yb, i % 3, l, p, "deduce_xline_block_group_block()"))
                rc = 1;
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_yline_block_group_block                                             */
/*----------------------------------------------------------------------------*/
int deduce_yline_block_group_block(Sudoku *s, int xb, int yb)
{
    int l;
    int rc;

    rc = 0;

    /* find unique premises within the block y line  */
    for(int p = 0; p < 9; p++)
    {
        l = -1;
        for(int xc = 0; xc < 3; xc++)
        {
            for(int yc = 0; yc < 3; yc++)
            {
                if(s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
                {
                    if(l == -1)
                    {
                        l = xc;
                        break;
                    }
                    else
                    {
                        l = -2;
                        break;
                    }
                }
            }
            if(l == -2)
                break;
        }
        if(l < 0)
            continue;

        Trace("  found b%d%d p%d x%d\n", xb, yb, p + 1, l);

        /* found double clear these in other cells */
        for(int i = 0; i < 9; i++)
        {
            if(yb == i / 3)
                continue;
            if(clear_premise(s, xb, i / 3, l, i % 3, p, "deduce_yline_block_group_block()"))
                rc = 1;
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_block_exclusive_group_cell                                          */
/*----------------------------------------------------------------------------*/
int deduce_block_exclusive_group_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    int c;
    int mask;
    int rc;

    rc = 0;

    /* find group unique premises within block */
    for(int size = 2; size < 8; size++)
    {
        c = 0;
        mask = 0;
        for(int p = 0; p < 9; p++)
        {
            if(s -> block_premises[xb][yb][p] == size && s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
            {
                mask |= s -> premise_bit[p];
                c++;
            }
        }
        if(c != size)
            continue;

        /* does the group exist */
        c = 0;
        for(int xc2 = xc; xc2 < 3; xc2++)
        {
            for(int yc2 = yc; yc2 < 3; yc2++)
            {
                if((s -> cell_premise[xb][yb][xc2][yc2] & mask) == mask)
                    c++;
            }
        }
        if(c != size)
            continue;

        #ifdef __TRACE__
        {
            int first;
        
            first = 1;
            printf("  found c%d%d%d%d ", xb, yb, xc, yc);
            for(int p = 0; p < 9; p++)
            {
                if(mask & s -> premise_bit[p])
                {
                    if(first)
                    {
                        printf("p%d", p + 1);
                        first = 0;
                    }
                    else printf("-p%d", p + 1);
                }
            }
            printf("\n");
        }
        #endif

        for(int xc2 = 0; xc2 < 3; xc2++)
        {
            for(int yc2 = 0; yc2 < 3; yc2++)
            {
                /* skip solved ones */
                if(s -> cell_premises[xb][yb][xc2][yc2] == 1)
                    continue;

                /* clear premises */
                for(int p = 0; p < 9; p++)
                {
                    if((s -> cell_premise[xb][yb][xc2][yc2] & mask) == mask)
                    {
                        /* clear non group premises within group members */
                        if(!(mask & s -> premise_bit[p]))
                        {
                            if(clear_premise(s, xb, yb, xc2, yc2, p, "deduce_block_exclusive_group_cell() group member"))
                                rc = 1;
                        }                        
                    }
                    else
                    {
                        /* clear group premises within non group members */
                        if(mask & s -> premise_bit[p])
                        {
                            if(clear_premise(s, xb, yb, xc2, yc2, p, "deduce_block_exclusive_group_cell() non group member"))
                                rc = 1;
                        }
                    }
                }
            }
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_block_group_cell                                                    */
/*----------------------------------------------------------------------------*/
int deduce_block_group_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    int c;
    int mask;
    int rc;

    /* skip solved cells */
    if(s -> cell_premises[xb][yb][xc][yc] == 1)
        return 0;

    /* find group premises within block */
    c = 0;
    mask = s -> cell_premise[xb][yb][xc][yc];
    for(int xc2 = xc; xc2 < 3; xc2++)
    {
        for(int yc2 = yc; yc2 < 3; yc2++)
        {
            if(mask == s -> cell_premise[xb][yb][xc2][yc2])
                c++;
        }
    }

    /* check if this is a group */
    if(s -> cell_premises[xb][yb][xc][yc] != c)
        return 0;

    #ifdef __TRACE__
    {
        int first;
    
        first = 1;
        printf("  found c%d%d%d%d ", xb, yb, xc, yc);
        for(int p = 0; p < 9; p++)
        {
            if(mask & s -> premise_bit[p])
            {
                if(first)
                {
                    printf("p%d", p + 1);
                    first = 0;
                }
                else printf("-p%d", p + 1);
            }
        }
        printf("\n");
    }
    #endif

    /* clear the group premises in other cells */
    rc = 0;
    for(int xc2 = 0; xc2 < 3; xc2++)
    {
        for(int yc2 = 0; yc2 < 3; yc2++)
        {
            /* skip solved ones */
            if(s -> cell_premises[xb][yb][xc2][yc2] == 1)
                continue;

            /* skip cells part of the group */
            if(s -> cell_premise[xb][yb][xc2][yc2] == mask)
                continue;

            /* clear premises */
            for(int p = 0; p < 9; p++)
            {
                if(mask & s -> premise_bit[p])
                {
                    if(clear_premise(s, xb, yb, xc2, yc2, p, "deduce_block_group_cell()"))
                        rc = 1;
                }
            }
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_xline_exclusive_group_cell                                          */
/*----------------------------------------------------------------------------*/
int deduce_xline_exclusive_group_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    int c;
    int mask;
    int rc;
   

    /* skip solved cells */
    if(s -> cell_premises[xb][yb][xc][yc] == 1)
        return 0;
    
    /* find group unique premises within x line */
    rc = 0;

    /* test all sizes, skip 1 and 9 */
    for(int size = 2; size < 9; size++)
    {
        c = 0;
        mask = 0;
        for(int p = 0; p < 9; p++)
        {
            if(s -> line_premises[X][yb * 3 + yc][p] == size && s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
            {
                mask |= s -> premise_bit[p];
                c++;
            }
        }
        if(c != size)
            continue;

        /* does the group exist */
        c= 0;
        for(int x = xb * 3 + xc; x < 9; x++)
        {
            if((s -> cell_premise[x / 3][yb][x % 3][yc] & mask) == mask)
                c++;
        }
        if(c != size)
            continue;

        #ifdef __TRACE__
        {
            int first;
        
            first = 1;
            printf("  found c%d%d%d%d ", xb, yb, xc, yc);
            for(int p = 0; p < 9; p++)
            {
                if(mask & s -> premise_bit[p])
                {
                    if(first)
                    {
                        printf("p%d", p + 1);
                        first = 0;
                    }
                    else printf("-p%d", p + 1);
                }
            }
            printf("\n");
        }
        #endif

        for(int x = 0; x < 9; x++)
        {
            /* skip solved ones */
            if(s -> cell_premises[x / 3][yb][x % 3][yc] == 1)
                continue;

            /* clear premises */
            for(int p = 0; p < 9; p++)
            {
                if((s -> cell_premise[x / 3][yb][x % 3][yc] & mask) == mask)
                {
                    /* clear non group premises within group members */
                    if(!(mask & s -> premise_bit[p]))
                    {
                        if(clear_premise(s, x / 3, yb, x % 3, yc, p, "deduce_xline_exclusive_group_cell() group member"))
                            rc = 1;
                    }                        
                }
                else
                {
                    /* clear group premises within non group members */
                    if(mask & s -> premise_bit[p])
                    {
                        if(clear_premise(s, x / 3, yb, x % 3, yc, p, "deduce_xline_exclusive_group_cell() non group member"))
                            rc = 1;
                    }
                }
            }
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_xline_groups                                                        */
/*----------------------------------------------------------------------------*/
int deduce_xline_group_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    int c;
    int mask;
    int rc;

    /* skip solved cells */
    if(s -> cell_premises[xb][yb][xc][yc] == 1)
        return 0;

    /* find group premises within x line */
    c = 0;
    mask = s -> cell_premise[xb][yb][xc][yc];
    for(int x = xb * 3 + xc; x < 9; x++)
    {
        if(mask == s -> cell_premise[x / 3][yb][x % 3][yc])
            c++;
    }

    /* check if this is a group */
    if(s -> cell_premises[xb][yb][xc][yc] != c)
        return 0;

    #ifdef __TRACE__
    {
        int first;
    
        first = 1;
        printf("  found cx%dx%d ", yb, yc);
        for(int p = 0; p < 9; p++)
        {
            if(mask & s -> premise_bit[p])
            {
                if(first)
                {
                    printf("p%d", p + 1);
                    first = 0;
                }
                else printf("-p%d", p + 1);
            }
        }
        printf("\n");
    }
    #endif

    /* clear the group premises in other cells */
    rc = 0;
    for(int x = 0; x < 9; x++)
    {
         /* skip solved ones */
         if(s -> cell_premises[x / 3][yb][x % 3][yc] == 1)
             continue;

          /* skip cells part of the group */
          if(s -> cell_premise[x / 3][yb][x % 3][yc] == mask)
              continue;

        /* clear premises */
        for(int p = 0; p < 9; p++)
        {
            if(mask & s -> premise_bit[p])
            {
                if(clear_premise(s, x / 3, yb, x % 3, yc, p, "deduce_xline_group_cell()"))
                    rc = 1;
        }
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_xwing_x                                                             */
/*----------------------------------------------------------------------------*/
int deduce_xwing_x(Sudoku *s)
{
    int i;
    int rc;
    int xw[6];

    Trace("deduce_xwing_x()\n");

    rc = 0;
    for(int p = 0; p < 9; p++)
    {
        for(int y1 = 0; y1 < 9; y1++)
        {
            if(s -> line_premises[X][y1][p] == 2)
            {
                /* find first line positions */
                xw[0] = y1;
                i = 1;
                for(int x1 = 0; x1 < 9; x1++)
                {
                    if(s -> cell_premise[x1 / 3][y1 / 3][x1 % 3][y1 % 3] & s->premise_bit[p])
                        xw[i++] = x1;
                }

                /* remove previous find and find second line positions */
                xw[4] = -1;
                for(int y2 = y1 + 1; y2 < 9; y2++)
                {
                    if(s -> line_premises[X][y2][p] == 2)
                    {
                        xw[3] = y2;
                        i = 4;
                        for(int x2 = 0; x2 < 9; x2++)
                        {
                            if(s -> cell_premise[x2 / 3][y2 / 3][x2 % 3][y2 % 3] & s->premise_bit[p])
                                xw[i++] = x2;
                        }
                    }
                }
                if(xw[1] == xw[4] && xw[2] == xw[5])
                {
                    Trace("  found: p:%d y1:%d x:%d,%d y2:%d x:%d,%d\n", p+1, xw[0], xw[1], xw[2], xw[3], xw[4], xw[5]);

                    /* clear premises in x */
                    for(int x = 0; x < 9; x++)
                    {
                        /* skip the xwing ones */
                        if(x == xw[1] || x == xw[2])
                            continue;
                        if(clear_premise(s, x / 3, xw[0] / 3, x % 3, xw[0] % 3, p, "deduce_xwing_y() x1"))
                            rc = 1;
                        if(clear_premise(s, x / 3, xw[3] / 3, x % 3, xw[3] % 3, p, "deduce_xwing_y() x2"))
                            rc = 1;
                    }

                    /* clear premises in y */
                    for(int y = 0; y < 9; y++)
                    {
                        /* skip the xwing ones */
                        if(y == xw[0] || y == xw[3])
                            continue;
                        if(clear_premise(s, xw[1] / 3, y / 3, xw[1] % 3, y % 3, p, "deduce_xwing_x() y1"))
                            rc = 1;
                        if(clear_premise(s, xw[2] / 3, y / 3, xw[2] % 3, y % 3, p, "deduce_xwing_x() y2"))
                            rc = 1;
                    }
                }
            }
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_xwing_y                                                             */
/*----------------------------------------------------------------------------*/
int deduce_xwing_y(Sudoku *s)
{
    int i;
    int rc;
    int xw[6];

    Trace("deduce_xwing_y()\n");

    rc = 0;
    for(int p = 0; p < 9; p++)
    {
        for(int x1 = 0; x1 < 9; x1++)
        {
            if(s -> line_premises[Y][x1][p] == 2)
            {
                /* find first line positions */
                xw[0] = x1;
                i = 1;
                for(int y1 = 0; y1 < 9; y1++)
                {
                    if(s -> cell_premise[x1 / 3][y1 / 3][x1 % 3][y1 % 3] & s->premise_bit[p])
                        xw[i++] = y1;
                }

                /* remove previous find and find second line positions */
                xw[4] = -1;
                for(int x2 = x1 + 1; x2 < 9; x2++)
                {
                    if(s -> line_premises[Y][x2][p] == 2)
                    {
                        xw[3] = x2;
                        i = 4;
                        for(int y2 = 0; y2 < 9; y2++)
                        {
                            if(s -> cell_premise[x2 / 3][y2 / 3][x2 % 3][y2 % 3] & s->premise_bit[p])
                                xw[i++] = y2;
                        }
                    }
                }
                if(xw[1] == xw[4] && xw[2] == xw[5])
                {
                    Trace("  found: p:%d x1:%d y:%d,%d x2:%d y:%d,%d\n", p+1, xw[0], xw[1], xw[2], xw[3], xw[4], xw[5]);

                    /* clear premises in x */
                    for(int x = 0; x < 9; x++)
                    {
                        /* skip the xwing ones */
                        if(x == xw[0] || x == xw[3])
                            continue;
                        if(clear_premise(s, x / 3, xw[1] / 3, x % 3, xw[1] % 3, p, "deduce_xwing_y() x1"))
                            rc = 1;
                        if(clear_premise(s, x / 3, xw[2] / 3, x % 3, xw[2] % 3, p, "deduce_xwing_y() x2"))
                            rc = 1;
                    }

                    /* clear premises in y */
                    for(int y = 0; y < 9; y++)
                    {
                        /* skip the xwing ones */
                        if(y == xw[1] || y == xw[2])
                            continue;
                        if(clear_premise(s, xw[0] / 3, y / 3, xw[0] % 3, y % 3, p, "deduce_xwing_x() y1"))
                            rc = 1;
                        if(clear_premise(s, xw[3] / 3, y / 3, xw[3] % 3, y % 3, p, "deduce_xwing_x() y2"))
                            rc = 1;
                    }
                }
            }
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_yline_exclusive_group_cell                                          */
/*----------------------------------------------------------------------------*/
int deduce_yline_exclusive_group_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    int c;
    int mask;
    int rc;

    /* skip solved cells */
    if(s -> cell_premises[xb][yb][xc][yc] == 1)
        return 0;

    /* find exclusive group unique premises within y line */
    rc = 0;

    /* test all sizes, skip 1 and 9 */
    for(int size = 2; size < 9; size++)
    {
        c = 0;
        mask = 0;
        for(int p = 0; p < 9; p++)
        {
            if(s -> line_premises[Y][xb * 3 + xc][p] == size && s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
            {
                mask |= s -> premise_bit[p];
                c++;
            }
        }
        if(c != size)
            continue;

        /* does the group exist */
        c= 0;
        for(int y = yb * 3 + yc; y < 9; y++)
        {
            if((s -> cell_premise[xb][y / 3][xc][y % 3] & mask) == mask)
                c++;
        }
        if(c != size)
            continue;

        #ifdef __TRACE__
        {
            int first;
        
            first = 1;
            printf("  found c%d%d%d%d ", xb, yb, xc, yc);
            for(int p = 0; p < 9; p++)
            {
                if(mask & s -> premise_bit[p])
                {
                    if(first)
                    {
                        printf("p%d", p + 1);
                        first = 0;
                    }
                    else printf("-p%d", p + 1);
                }
            }
            printf("\n");
        }
        #endif

        for(int y = 0; y < 9; y++)
        {
            /* skip solved ones */
            if(s -> cell_premises[xb][y / 3][xc][y % 3] == 1)
                continue;

            /* clear premises */
            for(int p = 0; p < 9; p++)
            {
                if((s -> cell_premise[xb][y / 3][xc][y % 3] & mask) == mask)
                {
                    /* clear non group premises within group members */
                    if(!(mask & s -> premise_bit[p]))
                    {
                        if(clear_premise(s, xb, y / 3, xc, y % 3, p, "deduce_yline_exclusive_group_cell() group member"))
                            rc = 1;
                    }                        
                }
                else
                {
                    /* clear group premises within non group members */
                    if(mask & s -> premise_bit[p])
                    {
                        if(clear_premise(s, xb, y / 3, xc, y % 3, p, "deduce_yline_exclusive_group_cell() non group member"))
                            rc = 1;
                    }
                }
            }
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* deduce_yline_groups                                                        */
/*----------------------------------------------------------------------------*/
int deduce_yline_group_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    int c;
    int mask;
    int rc;

    /* skip solved cells */
    if(s -> cell_premises[xb][yb][xc][yc] == 1)
        return 0;

    /* find group premises within y line */
    c = 0;
    mask = s -> cell_premise[xb][yb][xc][yc];
    for(int y = yb * 3 + yc; y < 9; y++)
    {
        if(mask == s -> cell_premise[xb][y / 3][xc][y % 3])
            c++;
    }

    /* check if this is a group */
    if(s -> cell_premises[xb][yb][xc][yc] != c)
        return 0;

    #ifdef __TRACE__
    {
        int first;
    
        first = 1;
        printf("  found c%dy%dy ", xb, xc);
        for(int p = 0; p < 9; p++)
        {
            if(mask & s -> premise_bit[p])
            {
                if(first)
                {
                    printf("p%d", p + 1);
                    first = 0;
                }
                else printf("-p%d", p + 1);
            }
        }
        printf("\n");
    }
    #endif

    /* clear the group premises in other cells */
    rc = 0;
    for(int y = 0; y < 9; y++)
    {
         /* skip solved ones */
         if(s -> cell_premises[xb][y / 3][xc][y / 3] == 1)
             continue;

          /* skip cells part of the group */
          if(s -> cell_premise[xb][y / 3][xc][y % 3] == mask)
              continue;

        /* clear premises */
        for(int p = 0; p < 9; p++)
        {
            if(mask & s -> premise_bit[p])
            {
                if(clear_premise(s, xb, y / 3, xc, y % 3, p, "deduce_yline_group_cell()"))
                    rc = 1;
            }
        }
    }

    return rc;
}

/*----------------------------------------------------------------------------*/
/* fill                                                                       */
/*----------------------------------------------------------------------------*/
int fill(Sudoku *s)
{
    int rc;
    int sudoku[9][9] = S;

    Trace("fill()\n");
    rc = 0;
    for(int x = 0; x < 9; x++)
    {
        for(int y = 0; y < 9; y++)
        {
            if(sudoku[y][x])
            {
                Trace("  found c%d%d%d%d p%d\n", x / 3, y / 3, x % 3, y % 3, sudoku[y][x]);
                for(int p = 0; p < 9; p++)
                {
                    /* clear all other premises within cell */
                    if(p != sudoku[y][x] - 1)
                    {
                        if(clear_premise(s, x / 3, y / 3, x % 3, y % 3, p, "fill()"))
                            rc = 1;
                    }
                }
            }
        }
    }
    return rc;
}

/*----------------------------------------------------------------------------*/
/* init                                                                       */
/*----------------------------------------------------------------------------*/
int init(Sudoku *s)
{
    Trace("init()\n");

    /* init cleared */
    s -> cleared = 0;

    /* init static premise premise_bits */
    for(int p = 0; p < 9; p++)
        s -> premise_bit[p] = 1 << p;
    
    /* init block_premises */
    for(int xb = 0; xb < 3; xb++)
    {
        for(int yb = 0; yb < 3; yb++)
        {
            for(int p = 0; p < 9; p++)
                s -> block_premises[xb][yb][p] = 9;
        }
    }

    /* init cell_premise and cell_premises */
    for(int xb = 0; xb < 3; xb++)
    {
        for(int yb = 0; yb < 3; yb++)
        {
            for(int xc = 0; xc < 3; xc++)
            {
                for(int yc = 0; yc < 3; yc++)
                {
                    s -> cell_premise[xb][yb][xc][yc] = AllPremiseBitsOn;
                    s -> cell_premises[xb][yb][xc][yc] = 9;
                }
            }
        }
    }

    /* init line_premises */
    for(int xy = 0; xy < 9; xy++)
    {
        for(int p = 0; p < 9; p++)
        {
            s -> line_premises[X][xy][p] = 9;
            s -> line_premises[Y][xy][p] = 9;
        }
    }

    return 0;
}

/*----------------------------------------------------------------------------*/
/* main                                                                       */
/*----------------------------------------------------------------------------*/
int main(void)
{
    Sudoku s;
    
    /* Initialize, print and fill */
    init(&s);
    print(&s);
    fill(&s);

    /* print and deduce until deduced */
    do
    {
        print(&s);
    }
    while (deduce(&s));
}

/*----------------------------------------------------------------------------*/
/* number                                                                     */
/*----------------------------------------------------------------------------*/
int number(int p)
{
    /* return the bit position number */
    if(p & 0x001)
        return 0;
    if(p & 0x002)
        return 1;
    if(p & 0x004)
        return 2;
    if(p & 0x008)
        return 3;
    if(p & 0x010)
        return 4;
    if(p & 0x020)
        return 5;
    if(p & 0x040)
        return 6;
    if(p & 0x080)
        return 7;
    if(p & 0x100)
        return 8;
    return -1;
}

/*----------------------------------------------------------------------------*/
/* print                                                                      */
/*----------------------------------------------------------------------------*/
int print(Sudoku *s)
{
    int p;

    Trace("print()\n");
    printf("\n");

    /* print sudoku */
    #ifdef __TRACE__
        /* print line statistics */
        for(int p = 0; p < 9; p++)
        {
            printf("          ");
            for(int x = 0; x < 9; x++)
            {
                if(x && x % 3 == 0)
                    printf("  ");
                printf(" %d", s -> line_premises[Y][x][p]);
            }
            printf("\n");
        }
        printf("         ");
    #endif
    printf(" -----------------------\n");
    for(int y = 0; y < 9; y++)
    {
        if(y && y % 3 == 0)
        {
            #ifdef __TRACE__
                printf("         ");
            #endif
            printf("|                       |\n");
        }
        #ifdef __TRACE__
            for(int p = 0; p < 9; p++)
            {
                printf("%d", s -> line_premises[X][y][p]);
            }
        #endif
        for(int x = 0; x < 9; x++)
        {
            if(x && x % 3 == 0)
                printf("  ");
            if(x == 0)
                printf("|");
            if(s -> cell_premises[x / 3][y / 3][x % 3][y % 3] == 1)
                printf(" %d", number(s -> cell_premise[x / 3][y / 3][x % 3][y % 3]) + 1);
            else
                printf(" .");
            if(x == 8)
                printf(" |");
        }
        printf("\n");
    }
    #ifdef __TRACE__
        printf("         ");
    #endif
    printf(" -----------------------\n");

    /* print cell statistics */
    #ifdef __TRACE__
        all_cells(s, print_cell);
        printf("\n");
    #endif
    return 0;
}

/*----------------------------------------------------------------------------*/
/* print_cell                                                                 */
/*----------------------------------------------------------------------------*/
int print_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    /* print block premises */
    if(xc == 0 && yc == 0)
    {
        printf("\n      ");
        for(int p = 0; p < 9; p++)
        {
            if(s -> block_premises[xb][yb][p])
                printf("%d", s -> block_premises[xb][yb][p]);
            else
                printf(".");
        }
        printf("\n");
    }    

    /* print cell premises */
    printf("%d%d%d%d: ", xb, yb, xc, yc);
    for(int p = 0; p < 9; p++)
    {
        if(s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
            printf("%d", p + 1);
        else
            printf(".");
    }

    /* print cell premise size or sudoku value */
    printf(" %d\n", s -> cell_premises[xb][yb][xc][yc]);

    return 0;
}

/*----------------------------------------------------------------------------*/
/* verify                                                                     */
/*----------------------------------------------------------------------------*/
int verify(Sudoku *s)
{
    /* check integrity */
    if(all_blocks(s, verify_block))
    {
        printf("Integrity lost in block_premises\n");
        exit(-1);
    }
    if(all_cells(s, verify_cell))
    {
        printf("Integrity lost in premises\n");
        exit(-1);
    }
    if(verify_xline(s))
    {
        printf("Integrity lost in line_premises[X]\n");
        exit(-1);
    }
    if(verify_yline(s))
    {
        printf("Integrity lost in line_premises[Y]\n");
        exit(-1);
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* verify_block                                                               */
/*----------------------------------------------------------------------------*/
int verify_block(Sudoku *s, int xb, int yb)
{
    int block_premises[9];

    for(int i = 0; i < 9; i++)
        block_premises[i] = 0;
    for(int xc = 0; xc < 3; xc++)
    {
        for(int yc = 0; yc < 3; yc++)
        {
            for(int p = 0; p < 9; p++)
            {
                if(s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
                    block_premises[p]++;
            }
        }
    }
    for(int p = 0; p < 9; p++)
    {
        if(block_premises[p] != s -> block_premises[xb][yb][p])
            return 1;
        if(block_premises[p] < 1 || block_premises[p] > 9)
            return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* verify_cell                                                                */
/*----------------------------------------------------------------------------*/
int verify_cell(Sudoku *s, int xb, int yb, int xc, int yc)
{
    int premises;

    premises = 0;
    for(int p = 0; p < 9; p++)
    {
        if(s -> cell_premise[xb][yb][xc][yc] & s -> premise_bit[p])
            premises++;
    }
    if(premises != s -> cell_premises[xb][yb][xc][yc])
        return 1;
    if(premises < 1 || premises > 9)
        return 1;
    return 0;
}

/*----------------------------------------------------------------------------*/
/* verify_xline                                                                */
/*----------------------------------------------------------------------------*/
int verify_xline(Sudoku *s)
{
    int line_premises[2][9][9];

    for(int y = 0; y < 9; y++)
    {
        for(int p = 0; p < 9; p++)
            line_premises[X][y][p] = 0;
    }
    for(int y = 0; y < 9; y++)
    {
        for(int p = 0; p < 9; p++)
        {
            for(int x = 0; x < 9; x++)
            {
                if(s -> cell_premise[x / 3][y / 3][x % 3][y % 3] & s -> premise_bit[p])
                    line_premises[X][y][p]++;
            }
        }
    }
    for(int y = 0; y < 9; y++)
    {
        for(int p = 0; p < 9; p++)
        {
            if(line_premises[X][y][p] != s -> line_premises[X][y][p])
                return 1;
            if(line_premises[X][y][p] < 1 || line_premises[X][y][p] > 9)
                return 1;
        }
    }
    return 0;
}

/*----------------------------------------------------------------------------*/
/* verify_yline                                                               */
/*----------------------------------------------------------------------------*/
int verify_yline(Sudoku *s)
{
    int line_premises[2][9][9];

    for(int x = 0; x < 9; x++)
    {
        for(int p = 0; p < 9; p++)
            line_premises[Y][x][p] = 0;
    }
    for(int x = 0; x < 9; x++)
    {
        for(int p = 0; p < 9; p++)
        {
            for(int y = 0; y < 9; y++)
            {
                if(s -> cell_premise[x / 3][y / 3][x % 3][y % 3] & s -> premise_bit[p])
                    line_premises[Y][x][p]++;
            }
        }
    }
    for(int x = 0; x < 9; x++)
    {
        for(int p = 0; p < 9; p++)
        {
            if(line_premises[Y][x][p] != s -> line_premises[Y][x][p])
                return 1;
            if(line_premises[Y][x][p] < 1 || line_premises[Y][x][p] > 9)
                return 1;
        }
    }
    return 0;
}
