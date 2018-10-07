//show local memory for values
void debug_values(State * state)
{
    
    printf("\n-----------------Executing-------------------");

    int width = 10;
    static Cursor * cursor;
    static byte * rgb;

    cursor = &state->cursor;
    

    dump_cursor(cursor);
    printf("\nCODE\t:%c ", state->code[state->pc].sym);
    printf("\nPC\t%d\n", state->pc);
    printf("\nCHANNEL\t%d\n", cursor->c);

    int i, j, x, y;
#if BF
    x = cursor->c - width / 2;
    if(x<0) x = 0;
    
    if (cursor->c + 5 > state->canvas_len)
        x -= cursor->c - state->canvas_len;
#else
    int cx = cursor->x,cy = cursor->y;

    x = cursor->x - width / 2;
    if(x<0) x = 0;
    y = cursor->y - width  / 2;
	if(y<0) y = 0;
    
    if (cursor->x + 5 > state->width)
        x -= cursor->x - state->width;
    if (cursor->y + 5 > state->height)
        y -= cursor->y - state->height;
#endif
    i = 0;
#if BF
    printf("% 3c ", ' ');
    while (i < width)
    {
        printf(" % 3d ", x + i);
        i++;
    }
    putchar('\n');
    i = 0;
    while (i < width)
    {
        printf("|% 3d|", state->canvas[x + i].value);
        i++;
    }
    putchar('\n');

#else
    printf("% 3c ", ' ');
    while (i < width)
    {
        printf(" % 3d ", x + i);
        i++;
    }
    putchar('\n');
    j = 0;
    while (j < width)
    {
        printf("% 3d ", y+j);
        state->cursor.y=y+j;
        i = 0;
        while (i <  width)
        {
            state->cursor.x=x+i;
            rgb = &state->canvas[INDEX].rgbt[0];
            printf("|% 3d|", rgb[cursor->c]);
            i++;
        }
        j++;
        putchar('\n');
    }
        state->cursor.x = cx;
        state->cursor.y = cy;
#endif
    getchar();
}