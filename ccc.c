/*
# ccc.c
Canvas, cursor, cells in C, cool?
Mix of Brainfuck and Turtle Graphics.

TODO
remove need for values?? treat as height x width x 3 matrix?
Spits out PPM IMage or ASCII or OpenGL context see usage.
Create A ahigh Level Macro Language (introduce variables)?

## Canvas  :  2D grid representation of display.
## Cell    :  Grid is composed of 4d vector defined as cells, (r,g,b,t).
                    - r,g,b represent the pixel [0-255],
                    - t is a “temp” pixel [0-255]. NOT used as an alpha component, just a temp register

## Cursor  :  Current cell being operated on, acts as tape head.

## Codes
.    |    write value as char to screen
,    |    Get users input from stdin (integer value)
>    |    Move forward
<    |    Move backward
+    |    increment current cell
-    |    decrement current cell
[    |    Start loop body, if v is not will continue operations,
]    |    Jumps back to matched opening brace if v is not 0
/    |    Rotate 90 degrees CW
\    |    Rotate 90 degrees CCW
{    |    Push cursor
}    |    Pop cursor
=    |    Set cursor to top stack
#    |    Write current canvas to output target
@    |    Clear canvas
x    |    set x to current cell
y    |    set y to current cell
r    |    switch to r channel
g    |    switch to g channel
b    |    switch to b channel
t    |    switch to t channel

//IDEAS

//Need?
Turn around (swap forward and backward) use // or \\


push digits to v? | (add integer codes)
wrap or no wrap for handling out of bounds???

# Usage TODO
cccc



# Initial Conditions
Starts at (0,0).
Forward is +x, Backward is -x

*/
//Implementation of BF to prove turing completeness by showing BF is subset of LAng, should be obvious though
//BF uses only values tape



#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> 

#define BF           0
#define DEBUG        1

#define ERR(msg,...) {printf("ERR:"msg,__VA_ARGS__); exit(0);}


//265*265 = 65536
#define DEFAULT_WIDTH           256
#define DEFAULT_HEIGHT          256
#define DEFAULT_CODE_LENGTH     65536
#define DEFAULT_BRACKET_DEPTH   2048
#define DEFAULT_FLAGS           (PPM | COUT)

//flags used to determine output
#define COUT    0x01    /*0000 0001*/
#define PPM     0x02    /*0000 0010*/

//#ANSII COLORS


#define FLAG(flags,option) ((flags) & (option))


#if BF
#define INDEX state->cursor.c
#else
#define INDEX (state->height*(state->cursor.y) + (state->cursor.x))
#endif


#if BF
#define CELL           state->canvas[INDEX].value
#else
#define CELL           state->canvas[INDEX].rgbt[cursor->c]
#endif



#define CASE(c,stmt) case c : if(!PARSE){stmt return 1;}


#define INPUT       CASE(',' ,   CELL = getchar();     )        
#define OUTPUT      CASE('.' ,    putchar(CELL);     )        
#define INC         CASE('+' ,   ++CELL;     )        
#define DEC         CASE('-' ,   --CELL;     )        
#define BEG_LOOP    CASE('[' ,   !CELL?state->pc = code->jump:0;      )        
#define END_LOOP    CASE(']' ,   CELL?state->pc = code->jump:0;      )        
#define FORWARD     CASE('>' ,   move_forward(state);      )        
#define BACKWARD    CASE('<' ,   move_backward(state);     )        
#define ROT_CW      CASE('/' ,   rot_cw(state);    )        
#define ROT_CCW     CASE('\\',   rot_ccw(state);     )        
#define PUSH_CUR    CASE('{' ,   /*todo*/     )        
#define POP_CUR     CASE('}' ,   /*todo*/     )        
#define PEEK_CUR    CASE('=' ,   /*todo*/     )        
#define DRAW        CASE('#' ,    draw(state);    )        
#define CLEAR       CASE('@' ,    memset(state->canvas, 0, sizeof(Cell) * state->canvas_len);       )        
#define SET_X       CASE('x' ,    state->cursor.x = CELL;    )        
#define SET_Y       CASE('y' ,    state->cursor.y = CELL;    )        
#define SET_R       CASE('r' ,    state->cursor.c = R;    )          
#define SET_G       CASE('g' ,    state->cursor.c = G;    )          
#define SET_B       CASE('b' ,    state->cursor.c = B;    )          
#define SET_T       CASE('t' ,    state->cursor.c = T;    )          





#define new(type, ptr, len)      \
{    int sz = sizeof(type) * len;\
    ptr = (type*)malloc(sz);     \
    if(ptr)memset(ptr, 0, sz);   \
}                                \



// HANDLE ERRORS BETTER!

typedef unsigned char    byte;
typedef int32_t          int32;
typedef struct _Code     Code;
typedef struct _Cell     Cell;
typedef struct _Cursor   Cursor;
typedef struct _State    State;

struct _Code
{
    byte sym;
    //if [ or ], jump is index to the matching brace
    int jump;
    int line, col;
};

struct _Cursor
{
    int    x, y;
    int    c;    //current channel
    int    dx,    dy;    //movement dir (1,0),(0,1),(-1,0),(0,-1)
};
//for opengl texture set stride to be sizeof(cell) and only read 3 rgb afor textture imagae data
//also, trying only loading texture once, se if modifying buffer will update on GPU
enum
{
    R=0, G=1, B=2, T=3
};
struct _Cell
{
    union
    {
        int32 value;
        byte rgbt[4];
    };
};

//TODO Create State Struct and dynamically alloc sizes +
// create a Vec class to handle this more effficiently and on the heap
//Global - no stack overflow
//             h      w

struct _State
{
    FILE *        file;    //defaults stdin
    Cell *        canvas;        //MAX_W*w+h
    Code *        code;       //op code
    Cursor        cursor;
    int pc;
    //set all below with args flags
    int code_len;
    int canvas_len;
    int bracket_depth;
    int width, height;
    //eg -ppm exports to ppm by |'ing to flags register
    byte flags;
};

//Main Functions
//byte parse(State * state);
//byte eval(State * state);

//void dump(Cell * cell)
//{
//#ifdef BF
//    printf("|%d|\n", cell->v);
//#else
//    printf("|%d|(%d,%d,%d)\n", cell->v, cell->r, cell->g, cell->b);
//#endif
//}





void dump_cursor(const Cursor * cursor)
{
#if BF
    printf("Cursor:%d\n", cursor->c);
#else
    puts("Cursor");
    printf("\tx  : %d\n", cursor->x);
    printf("\ty  : %d\n", cursor->y);
    printf("\tdx : %d\n", cursor->dx);
    printf("\tdy : %d\n", cursor->dy);
#endif
}

void dump(const State * state)
{
    puts("Options");
    printf("\tcode_len      : %d\n", state->code_len);
    printf("\tcanvas_len    : %d\n", state->canvas_len);
    printf("\tbracket_depth : %d\n", state->bracket_depth);
    puts("Canvas");
    printf("\twidth         : %d\n", state->width);
    printf("\theight        : %d\n", state->height);
}

void del_state(State * state)
{
    if (state->code)
        free(state->code);

    if (state->canvas)
        free(state->canvas);
}

void new_state(State * state, FILE * file, int height, int width, int code_len, int bracket_depth)
{
#if !( BF)
    state->cursor.x = 0;
    state->cursor.y = 0;
    state->cursor.dx = 1;
    state->cursor.dy = 0;
#endif
    state->cursor.c = 0;
    state->file = file;
    state->canvas_len = height*    width;
    state->code_len = code_len;
    state->bracket_depth = bracket_depth;
    state->width = width;
    state->height = height;
    state->flags = DEFAULT_FLAGS;


    //init new canvas
    new(Cell, state->canvas, state->canvas_len);
    if (state->canvas == 0)
        ERR("Could not allocate %d bytes for Canvas Memory!",  state->canvas_len*sizeof(Cell));
        
    //init new code tape
    new(Code, state->code, state->code_len);
    if (state->code == 0)
        ERR("Could not allocate %d bytes for Code Memory!", state->code_len*sizeof(Code));
        
    
}

byte move_forward(State * state)
{
    static    Cursor    *    cursor;
    cursor = &state->cursor;
#if BF
    ++(cursor->c);
    if (state->cursor.c >= state->canvas_len)
        ERR("Cursor Out of bounds: tape length=%d :  pos %d\n", state->canvas_len, state->cursor.c);
        
#else
    cursor->x += cursor->dx;
    cursor->y += cursor->dy;
    if (cursor->x >= state->width || cursor->y >= state->height)
        ERR("\nCursor Out of bounds: canvas size (%d, %d): Cursor at pos (%d, %d)\n", state->width, state->height, cursor->x, cursor->y);

#endif
    return 1;
}


byte move_backward(State * state)
{
    static    Cursor    *    cursor;
    cursor = &state->cursor;
#if BF
    --(cursor->c);
    if (state->cursor.c < 0)
    
        ERR("Cursor Out of bounds: tape length=%d :  pos %d\n", state->canvas_len, state->cursor.c);
#else       
    cursor->x -= cursor->dx;
    cursor->y -= cursor->dy;
    if (cursor->x < 0 || cursor->y < 0)
        ERR("\nCursor Out of bounds: canvas size (%d, %d): Cursor at pos (%d, %d)\n", state->width, state->height, cursor->x, cursor->y);
#endif
    return 1;
}


void rot_cw(State * state)
{
    static    Cursor    *    cursor;
    cursor = &state->cursor;
    if (cursor->dx)
    {
        cursor->dy = cursor->dx;
        cursor->dx = 0;
    }
    else
    {
        cursor->dx = -1 * cursor->dy;
        cursor->dy = 0;
    }
}

void rot_ccw(State * state)
{
    static    Cursor    *    cursor;
    cursor = &state->cursor;
    if (cursor->dx)
    {
        cursor->dy = cursor->dx*-1;
        cursor->dx = 0;
    }
    else
    {
        cursor->dx = cursor->dy;
        cursor->dy = 0;
    }
}


void draw(State * state)
{
    if (FLAG(state->flags, PPM))
    {
        static Cursor * cursor;
        static Cell * cell;

        puts("Exporting to PPM");
        //dump canvas too ppm
        FILE *file;
        file = fopen("out.ppm", "wb"); /* b - binary mode */
        fprintf(file, "P6\n%d %d\n255\n", state->width, state->height);

        int x, y;
        for (y = 0; y < state->height; ++y)
        {
            for (x = 0; x < state->width; ++x)
            {
                cell = &state->canvas[INDEX];
                fwrite(cell->rgbt, 1, 3, file);
            }
        }
        fclose(file);
    }
}





#define PARSE 1


//parse code
byte parse(State * state)
{
    static int * bracket_stack;
    if (!state) return 0;
    //index of the end of bracket stack (top is at bracket_index-1 )
    int        bracket_index = 0;
    Code *    code;
    int        sym;
    byte    valid_code;
    int        line = 0,col = 0;    //
    static Cursor * cursor;
    cursor = &state->cursor;

    bracket_stack = (int*)malloc(sizeof(int)*state->bracket_depth);

    state->pc = 0;
    while ((sym = fgetc(state->file)) != -1)
    {
        if (state->pc > state->code_len)
        {
            free(bracket_stack);
            ERR("Not enough Code Memory! Please shorten source code Max: %d", state->code_len);
        }
        col++;
        code = &state->code[state->pc];
        switch (sym)
        {
#if !(BF)
            ROT_CW   
            ROT_CCW  
            PUSH_CUR 
            POP_CUR  
            PEEK_CUR 
            DRAW     
            CLEAR    
            SET_X    
            SET_Y    
            SET_R    
            SET_G    
            SET_B    
            SET_T    
#endif
            INPUT    
            OUTPUT   
            INC      
            DEC      
            FORWARD  
            BACKWARD 
        {
            valid_code = 1;
            break;
        }
        //LOOP
        BEG_LOOP
        {
            bracket_stack[bracket_index++] = state->pc;
            valid_code = 1;
            break;
        }
        END_LOOP
        {

            if (bracket_index <= 0)
            {
                free(bracket_stack);
                ERR("Expecting Opening Bracket for bracket at line:%d, col:%d ", line, col);
                
            }
            int open_brack_index = bracket_stack[--bracket_index];
            code->jump = open_brack_index;
            //jump to next statement after closing bracket,
            state->code[open_brack_index].jump = state->pc + 1;
            valid_code = 1;
            break;
        }
        case '\n':
            //intentional fall-through because im mad at you :(
            line++;
        default:
            valid_code = 0;
            //skips any nonrecognized characters
            break;
        }//EndSwitch

        if (valid_code)
        {
            code->sym = sym;
            code->line = line;
            code->col = col;
            state->pc++;
        }
    }//End while

    if (bracket_index > 0)
    {

        free(bracket_stack);
        ERR("\nThe leading %d brackets are not closed", bracket_index);
    }

    //reset pc
    state->pc = 0;
    free(bracket_stack);

    return 1;

}


#define PARSE 0
byte eval(State * state)
{
    static    Code    *    code;
    static Cursor * cursor;
    cursor = &state->cursor;
    static    int            status;    //if an operation was not successful, some ops set to 0
    code = &state->code[state->pc];
    if(!code->sym)return 0; //NOOP
    state->pc++;
    switch (code->sym){
    INPUT
    OUTPUT
    INC
    DEC
    BEG_LOOP
    END_LOOP
    FORWARD
    BACKWARD
#if !(BF)
    ROT_CW
    ROT_CCW
    PUSH_CUR
    POP_CUR
    PEEK_CUR
    DRAW
    CLEAR
    SET_X
    SET_Y
    SET_R
    SET_G
    SET_B
    SET_T 
#endif
    }
    return 1;            //should not run Coorupted Code
}

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

int main(int argc, char ** argv)
{
    //init state syntax
    //-f value
    //file
    int width= DEFAULT_WIDTH,
        height = DEFAULT_HEIGHT,
        codelen = DEFAULT_CODE_LENGTH,
        bracket_depth = DEFAULT_BRACKET_DEPTH;
    FILE * file = stdin;
    int i;
    for (i = 1; i < argc; ++i)
    {
        printf("%s", argv[i]);
        getchar();

        if (strcmp(argv[i], "-"))
        {
            file = fopen(argv[i], "r+");
        }
    }

    State state;
    new_state
    (
        &state,
        file,
        width,
        height,
        codelen,
        bracket_depth
    );

    byte status = 1;
    if (parse(&state))
        while (status)
        {
#if DEBUG
			debug_values(&state);
#endif
            status = eval(&state);
        }

    del_state(&state);

    return 0;
}









