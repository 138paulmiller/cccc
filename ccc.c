
//enable runtime!
#define BF           1
#define DEBUG        0
//#define CURSOR_STACK        
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
//for opengl texture set stride to be sizeof(cell) and only read 3 rgb afor textture imagae data
//also, trying only loading texture once, se if modifying buffer will update on GPU



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



#define ERR(msg,...) {printf("ERR:"msg,__VA_ARGS__); exit(0);}


//265*265 = 65536
#define DEFAULT_WIDTH           256
#define DEFAULT_HEIGHT          256
#define DEFAULT_CODE_LENGTH     65536
#define DEFAULT_BRACKET_DEPTH   2048
#define DEFAULT_CURSOR_DEPTH   2048
#define DEFAULT_FLAGS           PPM 

//flags used to determine output
#define PPM     0x02    /*0000 0010*/

//#ANSII COLORS
#define FLAG(flags,option) ((flags) & (option))
//////////////////////// Structures /////////////////////////////////////////
enum{R=0, G=1, B=2, T=3};


typedef unsigned char    byte;
typedef int32_t          int32;

typedef struct
{
    byte sym;
    //if [ or ], jump is index to the matching brace
    int jump;
    int line, col;
}Code;

typedef struct 
{
    int    x, y;
    int    c;    //current channel
    int    dx,dy;    //movement dir (1,0),(0,1),(-1,0),(0,-1)
} Cursor;

typedef struct
{
    union   {int32 value;byte rgbt[4];};
}Cell;

typedef struct 
{
    FILE    *   file;    //defaults stdin
    Cell    *   canvas;        //MAX_W*w+h
    Code    *   code;       //op code
#ifdef CURSOR_STACK
    Cursor  *   cursors;  //TODO cursor stack
    int sp;  //cursor stack ptr
#else
    Cursor      cursor;  //TODO cursor stack    
#endif
    int  ip;  //instruction ptr, 


    //set all below with args flags
    int code_len;
    int canvas_len;
    int bracket_depth;
    int cursor_depth;
    int width, height;
    //eg -ppm exports to ppm by |'ing to flags register
    byte flags;
}State;
///////////////////// End structors


#ifdef CURSOR_STACK

    #define CURSOR state->cursors[state->sp]
#else
    #define CURSOR state->cursor
#endif

#define XY(x,y) state->width*x + y

#if BF
#define INDEX CURSOR.c
#else
#define INDEX XY(CURSOR.y,CURSOR.x)
#endif

#if BF
#define CELL           state->canvas[INDEX].value
#else
#define CELL           state->canvas[INDEX].rgbt[CURSOR.c]
#endif

#define CASE(c,stmt) case c : if(!PARSE){stmt return 1;}

#define COMMENT     ';'        
#define INPUT       CASE(',' ,   CELL = getchar();              )        
#define OUTPUT      CASE('.' ,    putchar(CELL);                )        
#define INC         CASE('+' ,   ++CELL;                        )        
#define DEC         CASE('-' ,   --CELL;                        )        
#define BEG_LOOP    CASE('[' ,   !CELL?state->ip = state->code[state->ip].jump:0;)        
#define END_LOOP    CASE(']' ,   CELL? state->ip = state->code[state->ip].jump:0; )        
#define FORWARD     CASE('>' ,   move_forward(state);           )        
#define BACKWARD    CASE('<' ,   move_backward(state);     )        
#define ROT_CW      CASE('/' ,   rot_cw(state);    )        
#define ROT_CCW     CASE('\\',   rot_ccw(state);     )        
#define PUSH_CUR    CASE('{' ,   /*todo*/     )        
#define POP_CUR     CASE('}' ,   /*todo*/     )        
#define PEEK_CUR    CASE('=' ,   /*todo*/     )        
#define DRAW        CASE('#' ,    draw(state);    )        
#define CLEAR       CASE('@' ,    memset(state->canvas, 0, sizeof(Cell) * state->canvas_len);       )        
#define SET_X       CASE('x' ,    CURSOR.x = CELL;    )        
#define SET_Y       CASE('y' ,    CURSOR.y = CELL;    )        
#define SET_R       CASE('r' ,    CURSOR.c = R;    )          
#define SET_G       CASE('g' ,    CURSOR.c = G;    )          
#define SET_B       CASE('b' ,    CURSOR.c = B;    )          
#define SET_T       CASE('t' ,    CURSOR.c = T;    )          


#if BF            
    #define RESET_CURSOR    CURSOR.c = 0;
#else
    #define RESET_CURSOR                 \
    CURSOR.x  = 0; \
    CURSOR.y  = 0; \
    CURSOR.dx = 1; \
    CURSOR.dy = 0; \
    CURSOR.c = 0;
#endif



#define BF_CODES INPUT OUTPUT INC DEC BEG_LOOP END_LOOP FORWARD BACKWARD
#if BF
    #define CASES   BF_CODES
#else
    #define CASES BF_CODES                              \
            ROT_CW      ROT_CCW                         \
            DRAW        CLEAR                           \
            SET_X       SET_Y                           \
            PUSH_CUR    POP_CUR     PEEK_CUR            \
            SET_R       SET_G       SET_B       SET_T   
#endif


#define new(type, ptr, len)      \
{    int sz = sizeof(type) * len;\
    ptr = (type*)malloc(sz);     \
    if(ptr)memset(ptr, 0, sz);   \
}                                


void dump_cursor(const State * state)
{
#if BF
    printf("Cursor:%d\n", CURSOR.c);
#else
    puts("Cursor");
    printf("\tx  : %d\n", CURSOR.x);
    printf("\ty  : %d\n", CURSOR.y);
    printf("\tdx : %d\n", CURSOR.dx);
    printf("\tdy : %d\n", CURSOR.dy);
#endif
}

void dump(const State * state)
{
    puts("Options");
    printf("\tcode_len      : %d\n", state->code_len);
    printf("\tcanvas_len    : %d\n", state->canvas_len);
    printf("\tbracket_depth : %d\n", state->bracket_depth);
    printf("\tcursor_depth : %d\n", state->cursor_depth);
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
#ifdef CURSOR_STACK
    if (state->cursors)
        free(state->cursors);
#endif
}

void new_state(State * state, FILE * file, int height, int width, int code_len, int bracket_depth)
{
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
#ifdef CURSOR_STACK
    new(Cursor, state->cursors, state->cursor_depth);
    if (state->cursors == 0)
        ERR("Could not allocate %d bytes for Code Memory!", state->cursor_depth*sizeof(Cursor));
    state->sp = -1;
#endif

    RESET_CURSOR       
    
}

byte move_forward(State * state)
{
#if BF
    ++(CURSOR.c);
    if (CURSOR.c >= state->canvas_len)
        ERR("Cursor Out of bounds: tape length=%d :  pos %d\n", state->canvas_len, CURSOR.c);
        
#else
    CURSOR.x += CURSOR.dx;
    CURSOR.y += CURSOR.dy;
    if (CURSOR.x >= state->width || CURSOR.y >= state->height)
        ERR("\nCursor Out of bounds: canvas size (%d, %d): Cursor at pos (%d, %d)\n", state->width, state->height, CURSOR.x, CURSOR.y);

#endif
    return 1;
}


byte move_backward(State * state)
{
#if BF
    --(CURSOR.c);
    if (CURSOR.c < 0)
    
        ERR("Cursor Out of bounds: tape length=%d :  pos %d\n", state->canvas_len, CURSOR.c);
#else       
    CURSOR.x -= CURSOR.dx;
    CURSOR.y -= CURSOR.dy;
    if (CURSOR.x < 0 || CURSOR.y < 0)
        ERR("\nCursor Out of bounds: canvas size (%d, %d): Cursor at pos (%d, %d)\n", state->width, state->height, CURSOR.x, CURSOR.y);
#endif
    return 1;
}


void rot_cw(State * state)
{
    if (CURSOR.dx)
    {
        CURSOR.dy = CURSOR.dx;
        CURSOR.dx = 0;
    }
    else
    {
        CURSOR.dx = -1 * CURSOR.dy;
        CURSOR.dy = 0;
    }
}

void rot_ccw(State * state)
{
    if (CURSOR.dx)
    {
        CURSOR.dy = CURSOR.dx*-1;
        CURSOR.dx = 0;
    }
    else
    {
        CURSOR.dx = CURSOR.dy;
        CURSOR.dy = 0;
    }
}


void draw(State * state)
{
    int i;
    if (FLAG(state->flags, PPM))
    {
        puts("Exporting to PPM");
        //dump canvas too ppm
        FILE *file;
        file = fopen("out.ppm", "wb"); /* b - binary mode */
        fprintf(file, "P6\n%d %d\n255\n", state->width, state->height);
        for (i = 0; i < state->canvas_len; ++i)
            fwrite(&state->canvas[i], 1, 3, file);

        fclose(file);
    }
}





#define PARSE 1
//parse code
byte parse(State * state)
{
    static int * bracket_stack;
    static Code     * code;
    if (!state) return 0;
    //index of the end of bracket stack (top is at bracket_index-1 )
    int        bracket_index = 0;
    int        sym;
    int        line = 0,col = 0;    //

    
    bracket_stack = (int*)malloc(sizeof(int)*state->bracket_depth);

    state->ip=0;
    while ((sym = fgetc(state->file)) != -1)
    {
        if (state->ip > state->code_len)
        {
            free(bracket_stack);
            ERR("Not enough Code Memory! Please shorten source code Max: %d", state->code_len);
        }
        col++;
        code = &state->code[state->ip];
        switch(sym)
        {
            BEG_LOOP
            {
                bracket_stack[bracket_index++] = state->ip;
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
                state->code[open_brack_index].jump = state->ip ;
                break;
            }
        }
        switch (sym)
        {
            case '\n':line++;break;
            CASES;
            {
            code->sym = sym;
            code->line = line;
            code->col = col;
            state->ip++;
            }
        }//EndSwitch

    }//End while
    if (bracket_index > 0)
    {

        free(bracket_stack);
        ERR("\nThe leading %d brackets are not closed", bracket_index);
    }
    //reset ip
#ifdef CURSOR_STACK
    state->sp = 0;
#endif
    free(bracket_stack);
    state->ip = 0;
    return 1;

}


#define PARSE 0
byte eval(State * state)
{
    switch (state->code[state->ip].sym){
        CASES
        ;default: return 0; //done NOOP 
    }
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
            status = eval(&state);
            ++state.ip;

#if DEBUG
            void debug_values(State * state);

            debug_values(&state);
#endif
        }

    del_state(&state);

    return 0;
}


#if DEBUG
///////////////////////////////////////////////// DEBUG /////////////////////////////////////////////////
//show local memory for values
void debug_values(State * state)
{
    
    printf("\n-----------------CURRENT STATE-------------------");

    int width = 10;
    static byte * rgb;
    dump_cursor(state);
    printf("\nCODE\t:%c ", state->code[state->ip].sym);
    printf("\nPC\t%d\n", state->ip);
    printf("\nCHANNEL\t%d\n", CURSOR.c);

    int i=0, j, x, y;
#if BF
    x = CURSOR.c - width / 2;
    if(x<0) x = 0;
    
    if (CURSOR.c + 5 > state->canvas_len)
        x -= CURSOR.c - state->canvas_len;
#else

    int cx = CURSOR.x, cy = CURSOR.y;
    x = CURSOR.x - width / 2;
    if(x<0) x = 0;
    y = CURSOR.y - width  / 2;
    if(y<0) y = 0;
    
    if (CURSOR.x + 5 > state->width)
        x -= CURSOR.x - state->width;
    if (CURSOR.y + 5 > state->height)
        y -= CURSOR.y - state->height;
#endif
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
        printf("% 3d ", y);
        CURSOR.y=y+j;
        i=0;
        while (i <  width)
        {
            CURSOR.x=x+i;
            rgb = &state->canvas[INDEX].rgbt[0];
            printf("|% 3d|", rgb[CURSOR.c]);
            i++;
        }
        j++;
        putchar('\n');
    }

    CURSOR.x=cx; CURSOR.y = cy;
#endif
    getchar();
}



#endif
