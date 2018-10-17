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
x    |    set current cell to x 
y    |    set current cell to y 
r    |    switch to r channel
g    |    switch to g channel
b    |    switch to b channel
t    |    switch to t channel
//ADd functionality to read into current cell the width and height of canvas??

//IDEAS

//Need?
Turn around (swap forward and backward) use // or \\


push digits to v? | (add integer codes)
wrap or no wrap for handling out of bounds???

# Usage TODO
cccc
//for opengl texture set stride to be sizeof(cell) and only read 3 rgb afor textture imagae data
//also, trying only loading texture once, se if modifying buffer will update on GPU
//USE PIXEL BUFFER!!


# Initial Conditions
Starts at (0,0).
Forward is +x, Backward is -x

*/
//Implementation of BF to prove turing completeness by showing BF is subset of LAng, should be obvious though
//BF uses only values tape

//Modify Mandlebrot to draw PPM

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> 

//Enable runtime configuration, use FLAGS for the openg/bf
//SEE MAKEFILE
//#define BF           0
//#define DEBUG        1
//#define OPENGL       1    /*set to 0 by default to prevent opengl build. remove*/

#define TEST_OPENGL 0

//265*265 = 65536
#define WIDTH           256
#define HEIGHT          256
#define CODE_LENGTH     65536/2
#define BRACKET_DEPTH   2048
#define CURSOR_DEPTH    2048
#define FLAGS           DRAW_OPENGL
// DRAW_PPM | 
//flags used to determine how data is output on #
#define DRAW_PPM         0x01    /*0000 0001*/
#define DRAW_OPENGL      0x02    /*0000 0010*/




#define EXIT(...) {printf(__VA_ARGS__); OPENGL_DESTROY;    free(canvas);     free(state->code);free(state); exit(0);}



//#ANSII COLORS
#define FLAG(flag) (state->flags & (flag))

#define CURSOR cursors[state->sp]

#define INDEX       state->height*CURSOR.y + CURSOR.x

#if BF
#define CELL        canvas[INDEX].data
#else
#define CELL        canvas[INDEX].rgbt[CURSOR.c]
#endif
#define CODE        state->code[state->ip]


#define CASE(c,stmt) case c : if(!PARSE){stmt; return 1;}

#define JUMP_SYM -1
#define MOVE_SYM -2

#define COMMENT          ';'        
#define JUMP        CASE(JUMP_SYM ,   state->ip = CODE.arg;) //skip is used by optimization pass to prevent reallocing array              
#define MOVE        CASE(MOVE_SYM  ,  move_cell()                             ) //skip is used by optimization pass to prevent reallocing array              
#define INPUT       CASE(',' ,   CELL = fgetc(stdin);                   )        
#define OUTPUT      CASE('.' ,   putchar(CELL)                          )        
#define INC         CASE('+' ,   CELL+=CODE.arg;                        )        
#define DEC         CASE('-' ,   CELL-=CODE.arg                       )        
#define BEG_LOOP    CASE('[' ,   if(!CELL)state->ip = CODE.arg)        
#define END_LOOP    CASE(']' ,   if(CELL)state->ip = CODE.arg )        
#define FORWARD     CASE('>' ,   move(CODE.arg)           )        
#define BACKWARD    CASE('<' ,   move(-1*CODE.arg)     )        
#define ROT_CW      CASE('/' ,   rot_cw()    )        
#define ROT_CCW     CASE('\\',   rot_ccw()     )        
#define PUSH_CUR    CASE('{' ,   if(++state->sp > CURSOR_DEPTH) EXIT("Cursor Stack Overflow",0)      )        
#define POP_CUR     CASE('}' ,   if(--state->sp < 0) EXIT("Cursor Stack Underflow",0)     )        
#define SET_CUR     CASE('=' ,        )        
#define DRAW        CASE('#' ,   draw()    )        
#define CLEAR       CASE('@' ,   memset(canvas, 0, sizeof(int32) * canvas_len)       )               
#define SET_R       CASE('r' ,   CURSOR.c = R   )          
#define SET_G       CASE('g' ,   CURSOR.c = G   )          
#define SET_B       CASE('b' ,   CURSOR.c = B   )          
#define SET_T       CASE('t' ,   CURSOR.c = T   )          

//USE SYMS FOR CAHNNELS to allow comments!!!!!
#define OPT_CODES JUMP MOVE
#define BF_CODES INPUT OUTPUT INC DEC BEG_LOOP END_LOOP FORWARD BACKWARD OPT_CODES
#if BF
    #define CASES   BF_CODES 
#else
    #define CASES BF_CODES                              \
            ROT_CW      ROT_CCW                         \
            DRAW        CLEAR                           \
            PUSH_CUR    POP_CUR                         \
            SET_R       SET_G       SET_B       SET_T   
#endif

//todo, 
#if OPENGL
    #include "gfx.h"

    #define  OPENGL_INIT            gl_init(state->width, state->height);\
                                    shader_init();\
                                    glUseProgram(m_program);\
                                    vao_init();\
                                    load_texture((byte*)&canvas[0], state->width, state->height);

    #define  OPENGL_DESTROY         vao_destroy(); \
                                    shader_destroy();\
                                    gl_destroy();                   
    
    #define  OPENGL_RENDER          gl_update();\
                                    gl_clear(); \
                                    update_texture((byte*)&canvas[0], 0,0, state->width, state->height);\
                                    vao_render();

 //gl_update(); load_texture(&canvas[0].rgbt[0], state->width,state->height); vao_render()
#else
    enum{ OPENGL_INIT, OPENGL_RENDER, OPENGL_DESTROY};
#endif 




#define new(type, ptr, len)      \
{    int sz = sizeof(type) * len;\
    ptr = (type*)malloc(sz);     \
    if(ptr)memset(ptr, 0, sz);else{EXIT("");}   \
}                                


//////////////////////// Structures /////////////////////////////////////////
enum{R=0, G=1, B=2, T=3};


typedef char    byte;
typedef int32_t          int32;

typedef struct
{
    byte sym;
    //if [ or ], jump is index to the matching brace
    int arg;
}Code;

typedef union{
    int32 data;
    byte rgbt[4];

}Cell;

typedef struct 
{
    int    x, y;
    int    minX,minY,maxX,maxY;
    int    c;    //current channel
    int    dx,dy;    //movement dir (1,0),(0,1),(-1,0),(0,-1)
}Cursor;
Cursor cursors[CURSOR_DEPTH];


typedef  struct 
{
    FILE    *   file;    //defaults stdin
    Code    *   code;       //op code
    int sp;  //cursor stack ptr
    int  ip;  //instruction ptr, 


    //set all below with args flags
    int code_len;
    int bracket_depth;
    int width, height;
    //eg -ppm exports to ppm by |'ing to flags register
    byte flags;
}State;
State *   state;

Cell  *    canvas;
int         canvas_len; //state->width * state->height


void move(int amt)
{
    CURSOR.x += CURSOR.dx*amt;
#if BF
    if (CURSOR.x >= state->width)
    {
        CURSOR.x = CURSOR.x-state->width;   
        CURSOR.y++;
    } 
#else
    CURSOR.y -= CURSOR.dy*amt;
#endif
    if (CURSOR.x >= state->width || CURSOR.y >= state->height || CURSOR.x < 0 || CURSOR.y < 0)
        EXIT("\nCursor Out of bounds: canvas size (%d, %d): Cursor at pos (%d, %d)\n", state->width, state->height, CURSOR.x, CURSOR.y);
}


void move_cell()
{
#if BF 
    int32
#else 
    byte
#endif
    * a = &CELL;
    move(1);
    CELL = *a;
    *a=  0;
    move(-1);
}



void rot_cw()
{
    if (CURSOR.dx)
    {
        CURSOR.dy = CURSOR.dx;
        CURSOR.dx = 0;
    }
    else
    {
        CURSOR.dx = CURSOR.dy*-1 ;
        CURSOR.dy = 0;
    }
}

void rot_ccw()
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


void draw()
{
    //FIX LOGIC to only update texture region

    CURSOR.minX = CURSOR.x;
    CURSOR.minY = CURSOR.y;
    CURSOR.maxX = CURSOR.x;
    CURSOR.maxY = CURSOR.y;

    int i;
    if (FLAG(DRAW_PPM))
    {
        FILE *file;
        file = fopen("out.ppm", "wb"); 
        fprintf(file, "P6\n%d %d\n255\n", state->width, state->height);
        for (i = 0; i < canvas_len; ++i)
            fwrite(&canvas[i], 1, 3, file);

        fclose(file);
    }
    if(FLAG(DRAW_OPENGL ))
    {
        OPENGL_RENDER;
    }
}





#define PARSE 1
//parse code
byte parse()
{
    static int bracket_stack[BRACKET_DEPTH];
    static Code     * code;
    if (!state) return 0;

    int        bracket_index = 0;
    int        sym,c;
    int        line = 0,col = 0;    
    state->ip=0;
    while ((sym = fgetc(state->file)) != -1)
    {
        if (state->ip > state->code_len)
            EXIT("Not enough Code Memory! Please shorten source code Max: %d", state->code_len);

        col++;
        code = &CODE;
     //special cases
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
                    EXIT("Expecting Opening Bracket for bracket at line:%d, col:%d ", line, col);
                int open_brack_index = bracket_stack[--bracket_index];
                code->arg = open_brack_index;
                //jump to next statement after closing bracket,
                state->code[open_brack_index].arg = state->ip ;
                break;
            };
            //eat same symbol, increment arg
            //do for push, rots as well
            INC DEC FORWARD BACKWARD
            {       
               do code->arg++;while(sym == (c=fgetc(state->file)));
                    //increment while same sym!, handle newline!!
               ungetc(c,state->file);//roll back to reeval
            }
        }
        switch (sym)
        {
            CASES
            {
                code->sym = sym;
                state->ip++;
            }
            //rmeove col and lines !!!
            case '\n': col=0; line++;    
        }//EndSwitch

    }//End while
    if (bracket_index > 0)
    {
        EXIT("\nThe leading %d brackets are not closed", bracket_index);
    }



    //reset ip
    state->sp = 0;
    state->ip = 0;
    
    return 1;

}

//Ad-hoc semantic_analysis
//nano optimization passes
#define NEXT_SYM  (code = &state->code[++ip])->sym

void optimize()
{
    //MOVE Optimize
    //translates all instance of [>+<-] pattern to tape[c] = tape[c+1];tape[c] =0
    //adds a skip  operand tothat will skip following 6 codes 
    
    int ip = 0;
    Code * code;
    do{
        switch(NEXT_SYM)
        {
            case '[':
                
                switch(NEXT_SYM)
                {
                    case '>':
                        switch(NEXT_SYM)
                        {

                            case '+':
                                switch(NEXT_SYM)
                                {

                                    case '<':
                                    switch(NEXT_SYM)
                                    {

                                        case '-':
                                         switch(NEXT_SYM)
                                            {

                                                case ']':
                                                code->sym = MOVE_SYM;
                                                (code=&state->code[ip-5])->sym = JUMP_SYM;
                                                code->arg= ip-1;

                                            }break;
                                    }break;

                                }break;

                        }break;
                }break;


        }
        /* code */
    }  while(code->sym);  

}

#define PARSE 0
byte eval()
{

    switch (CODE.sym){
        CASES
    }
    return 0; //done NOOP
}




int main(int argc, char ** argv)
{
    //init state syntax
    //-f value
    //file
    state = (State*)malloc(sizeof(State));
    state->file     = stdin;
    state->code_len = CODE_LENGTH;
    state->width    = WIDTH;
    state->height   = HEIGHT;
    state->flags    = FLAGS;
    state->sp       = 0;
    new(Code, state->code, state->code_len);
    CURSOR.c = 0;
    CURSOR.y  = HEIGHT-1;
    CURSOR.x  = CURSOR.dy = CURSOR.c =0;
    CURSOR.dx = 1; 

    canvas_len = state->width*state->height;
    new(Cell, canvas, canvas_len);

//    int i;
/*
    for (i = 1; i < argc; ++i)
        if (strcmp(argv[i], "-"))   
            state->file =fopen(argv[i], "r+");
*/  

    //init new canvas


       
    byte status = 1;

    OPENGL_INIT;        

    if (parse())
    {
        optimize();

        while   ( status)
        {
            status = eval();
            ++state->ip;
#if DEBUG
            void debug_values();
            debug_values();
#endif 
        }
    }
    //calls destroy and exits
    EXIT("");

}


#if DEBUG

///////////////////////////////////////////////// DEBUG /////////////////////////////////////////////////
//show local memory for values
void debug_values()
{
    
    printf("\n-----------------CURRENT STATE-------------------");

    int tab = 10, htab = 5;
    static byte * rgb;
    puts("Cursor");
    printf("\tx  : %d\n", CURSOR.x);
    printf("\ty  : %d\n", CURSOR.y);
    printf("\tdx : %d\n", CURSOR.dx);
    printf("\tdy : %d\n", CURSOR.dy);
    printf("\nCODE\t:%c ", CODE.sym);
    printf("\nARG\t:%d ", CODE.arg);
    printf("\nPC\t%d\n", state->ip);
    printf("\nCHANNEL\t%d\n", CURSOR.c);

    int i=0, j, x, y;
    int cx = CURSOR.x, cy = CURSOR.y;
    x = CURSOR.x - htab;
    if(x<0) x = 0;
    y = CURSOR.y - htab;
    if(y<0) y = 0;
    
    if (x + tab > state->width)
        x -= (x+tab) - state->width;
    if (y + tab > state->height)
        y -= (y+tab) - state->height;
    printf("% 3c ", ' ');
    while (i < tab)
    {
        printf(" % 3d ", x + i);
        i++;
    }
    putchar('\n');
    j = 0;
    while (j < tab)
    {
        CURSOR.y=y+j;

        printf("% 3d ", CURSOR.y);
        i=0;
        while (i <  tab)
        {
            CURSOR.x=x+i;
            rgb = &canvas[INDEX].rgbt[0];
            printf("|% 3d|", rgb[CURSOR.c]);
            i++;
        }
        j++;
        putchar('\n');
    }

    CURSOR.x=cx; CURSOR.y = cy;
    //getchar();
}
#endif 


