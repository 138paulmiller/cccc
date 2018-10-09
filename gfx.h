#include <GL/glew.h>
#include <SDL2/SDL.h>
SDL_Window*     m_sdlWindow;
SDL_Event*      m_sdlEvent;
SDL_GLContext   m_glContext;

static float verts[6*2] = {
    0.0, 0.0,
    0.0, 1.0,
    1.0, 0.0,

    0.0, 0.0,
    1.0, 1.0,
    0.0, 1.0
};

//grab canvas data as 2d texture

void gl_init(int width, int height)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        puts("SDL Failed to initialize!"); exit(0);     
    }        
    
    m_sdlWindow = SDL_CreateWindow("cccc",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height, SDL_WINDOW_OPENGL);
    //Initialize opengl color attributes buffer size
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 2);

    m_glContext = SDL_GL_CreateContext(m_sdlWindow);
    if(glewInit() != GLEW_OK){
            puts("GLEW Failed to initialize");exit(0);

    }   
}


void gl_destroy()
{
    //destroy context then window
    SDL_GL_DeleteContext(m_glContext);
    SDL_DestroyWindow(m_sdlWindow);
    SDL_Quit();
}

void gl_clear()
{
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
}


void gl_update()
{
  //swap render and display buffers
    SDL_GL_SwapWindow(m_sdlWindow);

    //Poll input events
    SDL_Event event;
    while (SDL_PollEvent(&event));

}
GLuint m_vert_shader, m_frag_shader, m_program;

GLuint make_shader(GLuint type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;

}

void shader_init()
{
    puts("Shader INIT ...\n");
    const char * vs = 
    "#version 130   "
    "in vec2 pos;"
    "void main(){gl_Position =vec4(pos,1,1);}";


    const char * fs = 
    "#version 130   "
    "void main(){gl_FragColor=gl_FragCoord;}";



    m_program = glCreateProgram();
    glAttachShader(m_program, make_shader(GL_VERTEX_SHADER, vs) );
    glAttachShader(m_program, make_shader(GL_FRAGMENT_SHADER,fs) );
    glLinkProgram(m_program);
    glValidateProgram(m_program);

}

void shader_destroy()
{
    glDeleteShader(m_frag_shader); // removes
    glDeleteProgram(m_program);

}
int m_vao, m_vbo;

void vao_init()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glUseProgram(m_program); //Attaching shaders
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);

}



void vao_destroy()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);

}

//bind to use
void vao_render() 
{

    //bind the VAO and its element buffer
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0,  6);
}



//bind to use
void load_texture(char * data, int w, int h) 
{
    glUseProgram(m_program); //Attaching shaders

}

