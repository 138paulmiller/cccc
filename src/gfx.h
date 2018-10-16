#include <GL/glew.h>
#include <SDL2/SDL.h>
SDL_Window*     m_sdlWindow;
SDL_Event*      m_sdlEvent;
SDL_GLContext   m_glContext;
int m_texture, m_vert_shader, m_frag_shader, m_program;
int m_vao, m_vbo;

static float verts[12] = {
    -1.0, -1.0,
    -1.0, 1.0,
    1.0, 1.0,

    -1.0, -1.0,
    1.0, -1.0,
    1.0, 1.0,
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
    SDL_DestroyWindow(m_sdlWindow);
    SDL_GL_DeleteContext(m_glContext);
    SDL_Quit();
}

void gl_clear()
{
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
}


int gl_update()
{
  //swap render and display buffers
    SDL_GL_SwapWindow(m_sdlWindow);

    //Poll input events
    SDL_Event event;
    while (SDL_PollEvent(&event))
        if (event.type == SDL_QUIT) return 0;
    return 1;

}

int make_shader(GLuint type, const char* source)
{
    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    checkError(shader, GL_COMPILE_STATUS,0);
    return shader;

}

void shader_init()
{
    const char * vs = 
    "#version 130\n"
    "in vec2 pos;"
    "out vec2 uv;"
    "void main(){"
        "uv=pos*0.5+0.5;"
        "gl_Position  =vec4(pos,1,1);\n"

    "}";


    const char * fs = 
    "#version 130\n"
    "uniform sampler2D sampler;"
    "in vec2 uv;"
    "out vec4 color;"
    "void main(){\n"
        "color = texture(sampler,uv);\n"
//        "color = vec4(uv,0,1);\n"
    "}";

    m_program = glCreateProgram();
    puts("Vert...");
    glAttachShader(m_program, make_shader(GL_VERTEX_SHADER, vs) );
    puts("Frag....");
    glAttachShader(m_program, make_shader(GL_FRAGMENT_SHADER,fs) );
    glLinkProgram(m_program);
    if(checkError(m_program, GL_LINK_STATUS, 1))
        return;
    glValidateProgram(m_program);
    if(checkError(m_program, GL_VALIDATE_STATUS, 1))return;


}

int checkError(int  shader, int  flag, int isProgram)//////////////////remove eventually
{
    int status = 0;
    isProgram ?
        glGetProgramiv(shader, flag, &status)
        : glGetShaderiv(shader, flag, &status);
    if (status == GL_FALSE)
    {
        int errorLen = 0;
        isProgram ?
            glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &errorLen)
            : glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorLen);
        char* errorMsg = (char*)malloc(errorLen);
        isProgram ?
            glGetProgramInfoLog(shader, errorLen, &errorLen, errorMsg)
            : glGetShaderInfoLog(shader, errorLen, &errorLen, errorMsg);
        printf("Shader Log: %d %s \n", errorLen, errorMsg);
        free( errorMsg);
        return 0;
    }
    return 1;
}

void shader_destroy()
{
    glDeleteShader(m_frag_shader); // removes
    glDeleteProgram(m_program);

}

void vao_init()
{
    glUseProgram(m_program); //Attaching shaders
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);


    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), 0);
    glEnableVertexAttribArray(0);//enabel to draw
  
    glGenTextures(1, &m_texture);
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
    //glActiveTexture(GL_TEXTURE0); 
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glDrawArrays(GL_TRIANGLES, 0,  6);
}



//bind to use
void load_texture(GLubyte * data, int w, int h) 
{ 
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)data);
    //glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)data);

}

void update_texture(GLubyte * data, int xoff, int yoff, int w, int h)
{
    glTexSubImage2D(  GL_TEXTURE_2D, 0,xoff,yoff,w,h,GL_RGBA,GL_UNSIGNED_BYTE,data);
}


int test_opengl()
{
    gl_init(500,500);
    shader_init(); 
    vao_init();
    int width=256,height=256;
    int len = width * height*4;
    GLubyte * texture = (GLubyte *  )malloc(len);
    //memset(&texture[0], 120, len);

    int i,j,x;
    for(j=0; j < height; j++)
        for(i=0; i < width; i++)
        {
            x = 4*j*height+i*4;
            texture[x++] =  255;
            texture[x++] =  0;
            texture[x] =  0;
        }

    while(gl_update())
    {
        gl_clear();
        glUseProgram(m_program); //Attaching shaders

        load_texture(&texture[0], width,height); 
        vao_render();
    }   
    shader_destroy(); 
    vao_destroy();
    gl_destroy(); 
    return 0;

}