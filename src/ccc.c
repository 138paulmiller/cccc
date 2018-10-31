//ccc.c
//github.com/138paulmiller
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h> 

//Enable runtime configuration, use FLAGS for the openg/bf
//SEE MAKEFILE
//#define BF        0
//#define OPENGL    1    /*set to 0 by default to prevent opengl build. remove*/
//#define DEBUG       0
 #define OPTIMIZE   1

#define TEST_OPENGL 0

#define WRAP_MEMORY 0 //wrap memory index boundaries
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


//2D grid - push pixel to ither rgb or a channel

#define EXIT(...) { printf(__VA_ARGS__); \
                    OPENGL_DESTROY;\
                    free(canvas);  \
                    free(memory); \
                    free(state->code);\
                    free(state); \
                    exit(0);\
                    }
//DOUBLE BUFFER! Splits redraw to pushpixels and clear canvas commands. pushpixels to canvas!!!!


//#ANSII COLORS
#define FLAG(flag) (state->flags & (flag))

#define INDEX       state->height*CURSOR.y+CURSOR.x      
#define CELL        memory[INDEX]
#define PIXEL       canvas[INDEX].rgb[ CURSOR.c]
#define CODE        state->code[state->ip]
#define CURSOR      cursors[state->sp]

#define PRINT_CURSOR(c) printf("(%d %d) d(%d %d)\n", c.x, c.y, c.dx, c.dy);


#define CASE(c,stmt) case c : if(!PARSE){stmt; return 1;}

#define PUSH_CUR_STACK  memcpy(&cursors[state->sp+1],&cursors[state->sp],sizeof(Cursor));\
                        state->sp++;


#define CHECK_CURSOR_STACK  if(state->sp > CURSOR_DEPTH) EXIT("Cursor Stack Overflow")   \
                            else if(state->sp < 0) EXIT("Cursor Stack Underflow")       


#define CHECK_CHANNEL    if(state->sp > 3) EXIT("Channel Overflow")   \
                            else if(state->sp < 0) EXIT("Channel Underflow")       

#define CLEAR_CANVAS memset(canvas,0,sizeof(byte)*3*memory_len)

//Optimized instructions
//Many instructions are collaped into a single instruction and a jump
#define JUMP_SYM 255-0
#define MOVE_SYM 255-1
#define ZERO_SYM 255-2


//up to 255
#define COMMENT          ';'        
#define JUMP            CASE(JUMP_SYM ,  state->ip = CODE.arg              ) //skip is used by optimization pass to prevent reallocing array              
#define MOVE            CASE(MOVE_SYM ,  move_cell()                       ) //skip is used by optimization pass to prevent reallocing array              
#define ZERO_OUT        CASE(ZERO_SYM ,  CELL=0                            ) //skip is used by optimization pass to prevent reallocing array              
#define INPUT           CASE(','      ,  CELL = fgetc(stdin)               )        
#define OUTPUT          CASE('.'      ,  output()                          ) //push pixel if BF        
#define INC             CASE('+'      ,  CELL+=CODE.arg                    )        
#define DEC             CASE('-'      ,  CELL-=CODE.arg                    )        
#define BEG_LOOP        CASE('['      ,  if(!CELL)state->ip = CODE.arg     )        
#define END_LOOP        CASE(']'      ,  if(CELL)state->ip = CODE.arg      )        
#define FORWARD         CASE('>'      ,  move(CODE.arg)                    )        
#define BACKWARD        CASE('<'      ,  move(-1*CODE.arg )                )        
#define ROT_CW          CASE('/'      ,  rot_cw()                          )        
#define ROT_CCW         CASE('\\'     ,  rot_ccw()                         )        
#define PUSH_CUR        CASE('('      ,  PUSH_CUR_STACK CHECK_CURSOR_STACK )
#define POP_CUR         CASE(')'      ,  state->sp--; CHECK_CURSOR_STACK    )      
#define DRAW            CASE('#'      ,  draw()                            )        
#define CLEAR           CASE('@'      ,  CLEAR_CANVAS                      )               
#define CHANNEL_NEXT    CASE('}'      ,  CURSOR.c+=CODE.arg; CHECK_CHANNEL )          
#define CHANNEL_PREV    CASE('{'      ,  CURSOR.c-=CODE.arg; CHECK_CHANNEL ) 
//USE SYMS FOR CAHNNELS to allow comments!!!!!
#define OPT_CODES JUMP MOVE ZERO_OUT
#define BF_CODES INPUT OUTPUT INC DEC BEG_LOOP END_LOOP FORWARD BACKWARD OPT_CODES
#if BF
    #define CASES   BF_CODES 
#else
    #define CASES BF_CODES                              \
            ROT_CW      ROT_CCW                         \
            DRAW        CLEAR                           \
            PUSH_CUR    POP_CUR                         \
            CHANNEL_NEXT       CHANNEL_PREV   
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
    if(ptr)memset(ptr, 0, sz);else{EXIT(" ");}   \
}                                


//////////////////////// Structures /////////////////////////////////////////
enum{R=0, G=1, B=2, T=3};


typedef uint32_t  uint32;
typedef int32_t  int32;
typedef unsigned char     byte;
typedef uint32_t     cell;

typedef struct
{
    byte sym;
    //if [ or ], jump is index to the matching brace
    int arg;
}Code;


typedef struct 
{
    uint32    x, y;
    uint32   c;    //current channel
    byte    dx,dy;    //movement dir (1,0),(0,1),(-1,0),(0,-1)
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


struct
{
    byte rgb[3]; 
}   *    canvas;


cell  *    memory;
int         memory_len; //state->width * state->height
int    minX,minY,maxX,maxY;


void output()
{
#if BF
    putchar(CELL);
#else
//write pixel to texture 
    PIXEL = CELL;
#endif

}

void move(int amt)
{
    CURSOR.x += CURSOR.dx*amt;
#if BF
    if (CURSOR.x >= memory_len || CURSOR.x < 0)
        EXIT("\nCursor Out of bounds: canvas len %d: Cursor at pos %d\n", memory_len, CURSOR.x);
#else 
    CURSOR.y += CURSOR.dy*amt;
    if (CURSOR.x >= state->width || CURSOR.y >= state->height || CURSOR.x < 0 || CURSOR.y < 0)
        EXIT("\nCursor Out of bounds: canvas size (%d, %d): Cursor at pos (%d, %d)\n", state->width, state->height, CURSOR.x, CURSOR.y);
#endif
#if WRAP_MEMORY
    if (CURSOR.x >= state->width)
        CURSOR.x%=state->width;
    if (CURSOR.y >= state->height)
        CURSOR.y%=state->height;

#else 
#endif
}


void move_cell()
{
    cell
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

    if(CURSOR.x < minX) minX = CURSOR.x;
    else 
    if(CURSOR.x > maxX) maxX = CURSOR.x;
    if(CURSOR.y < minY) minY = CURSOR.y;
    else 
    if(CURSOR.y > maxY) maxY = CURSOR.y;

    int i;
    if (FLAG(DRAW_PPM))
    {
        FILE *file;
        file = fopen("out.ppm", "wb"); 
        fprintf(file, "P6\n%d %d\n255\n", state->width, state->height);
        for (i = 0; i < memory_len; ++i)
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
            INC DEC FORWARD BACKWARD CHANNEL_NEXT CHANNEL_PREV
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
//ip index of symbol to jummp from. jumps to "this" ip 
#define SET_JUMP(new_sym, ip_off)   code->sym = new_sym; (code=&state->code[ip-ip_off])->sym = JUMP_SYM;code->arg= ip-1;
#define switch_sym  switch(NEXT_SYM) 
void optimize()
{
    //MOVE Optimize
    //translates all instance of [>+<-] pattern to tape[c] = tape[c+1];tape[c] =0
    //adds a skip  operand tothat will skip following 6 codes 
    //add one for zeroing out [-] as well as copy
    int ip = 0;
    Code * code;
    do{
        switch_sym
        {
            case '[':   
                switch_sym
                {
                    case '>':
                        switch_sym
                        {

                            case '+':
                                switch_sym
                                {

                                    case '<':
                                    switch_sym
                                    {

                                        case '-':
                                         switch_sym
                                            {

                                                case ']':
                                                SET_JUMP(MOVE_SYM,5) //[>+<-] == 5 syms
                                                break; //?optional break for now; end case ]
                                            }break;//end case -
                                    }break;//end case <

                                }break; //end case +

                        }break; //end case >
                    case '-':
                        switch_sym{
                            case ']':   //case [-] == zsero out
                            SET_JUMP(ZERO_SYM, 2)
                            break;
                        }break;
                }break; //end case [


        }
        /* code */
    }  while(code->sym);  

}

#define PARSE 0
byte eval()
{

    //printf("\nCODE: %c | CELL:%d", CODE.sym, CELL);
    switch (CODE.sym)
    {
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
    CURSOR.y  = 
    0
;
    CURSOR.x  = CURSOR.dy = CURSOR.c =0;
    CURSOR.dx = 1; 
    minX = state->width;
    minY = state->height; 
    maxX = -1;
    maxY = -1; 

    memory_len = state->width*state->height;
    new(byte, canvas, 3*memory_len);
    
    new(cell, memory, memory_len);

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
        #if OPTIMIZE
        optimize();
        #endif
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
    EXIT(" ");

}


#if DEBUG

///////////////////////////////////////////////// DEBUG /////////////////////////////////////////////////
//show local memory for values
void debug_values()
{
 //if(-1==getchar())return;   

    //printf("\n-----------------CURRENT STATE-------------------\n");
    if(-1==getchar())return;
    int tab = 5, htab = tab/2;
/*
    puts("Cursor");
    printf("\tx  : %d\n", CURSOR.x);
    printf("\ty  : %d\n", CURSOR.y);
    printf("\tdx : %d\n", CURSOR.dx);
    printf("\tdy : %d\n", CURSOR.dy);
    printf("\nCODE\t:%c ", CODE.sym);
    printf("\nARG\t:%d ", CODE.arg);
    printf("\nPC\t%d\n", state->ip);
    printf("\nCHANNEL\t%d\n", CURSOR.c);
*/

    printf("\n %c:%d", CODE.sym,  CODE.arg);    
    printf("\t(%2d %2d)\n", CURSOR.x, CURSOR.y);
    
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

            printf("|% 3d|", CELL);
            i++;
        }
        j++;
        putchar('\n');
    }

    CURSOR.x=cx; CURSOR.y = cy;
    //getchar();
    usleep(2);
}
#endif 


