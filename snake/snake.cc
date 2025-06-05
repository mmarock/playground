

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <glm/ext/matrix_float4x4.hpp>
#include <stdexcept>
#include <utility>
#include <vector>

#include <pstl/glue_algorithm_defs.h>
#include <unistd.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <GL/glew.h>
//#include <glad/gl.h>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"


using vec2 = std::pair<uint8_t, uint8_t>;

struct graphics;
struct snake;
struct world;

struct isnake
{
    virtual void up() = 0;
    virtual void down() = 0;
    virtual void left() = 0;
    virtual void right() = 0;
};

struct app
{
    bool ready{false};
    graphics *g{nullptr};
    isnake *s{nullptr};
    world *w{nullptr};
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    fprintf(stdout, "pressed key %d\n", key);

    app * a = (app *)glfwGetWindowUserPointer(window);

    if(a && a->s)
    {
        switch(key)
        {
            case GLFW_KEY_UP:
                a->s->up();
                break;
            case GLFW_KEY_DOWN:
                a->s->down();
                break;
            case GLFW_KEY_LEFT:
                a->s->left();
                break;
            case GLFW_KEY_RIGHT:
                a->s->right();
                break;
        }

    }
}

static const char * vertex_shader_text = R"(
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 MVP;
uniform mat4 TEX_MVP;

out vec2 texCoord;

void main()
{
    gl_Position = MVP * vec4(aPos, 0.0, 1.0);

    texCoord = (TEX_MVP * vec4(aTexCoord, 0.0, 1.0)).xy;
}
)";

static const char * fragment_shader_text = R"(
#version 330 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, texCoord);
}
)";

struct graphics
{
    // 
    constexpr static uint8_t height_pixel = 160;
    constexpr static uint8_t width_pixel = 144;

    // 160 / 8 = 20
    // 144 / 8 = 18

    GLFWwindow *window_{nullptr};
    GLuint program_{0};

    GLuint mvp_handle_{0};

    GLuint tex_mvp_{0};

    // setup the libs, create a window
    graphics(app *app)
    {
        glfwSetErrorCallback(&error_callback);

        glfwInit();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(width_pixel, height_pixel, "Wait for the snake", NULL, NULL);
        if(!window_)
        {
            throw std::runtime_error("create window error");
        }

        glfwMakeContextCurrent(window_);

        glewInit();

        // compile the shader
        GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vshader, 1, &vertex_shader_text, NULL);
        glCompileShader(vshader);

        int suc=0;
        char log[512];
        glGetShaderiv(vshader, GL_COMPILE_STATUS, &suc);
        if(!suc)
        {
            glGetShaderInfoLog(vshader, sizeof(log), NULL, log);
            printf("vertex shader: %s\n", log);
        }

        GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fshader, 1, &fragment_shader_text, NULL);
        glCompileShader(fshader);

        glGetShaderiv(fshader, GL_COMPILE_STATUS, &suc);

        if(!suc)
        {
            glGetShaderInfoLog(fshader, sizeof(log), NULL, log);
            printf("fragment shader: %s\n", log);
        }

        printf("got shader\n");
        
        program_ = glCreateProgram();
        glAttachShader(program_, vshader);
        glAttachShader(program_, fshader);
        glLinkProgram(program_);

        glGetProgramiv(program_,GL_LINK_STATUS, &suc);
        if(!suc)
        {
            glGetProgramInfoLog(program_, 512, NULL, log);
            printf("program: %s\n", log);
        }

        mvp_handle_ = glGetUniformLocation(program_, "MVP");

        tex_mvp_ = glGetUniformLocation(program_, "TEX_MVP");

        glDeleteShader(vshader);
        glDeleteShader(fshader);

        // TODO set before render?
        glUseProgram(program_);

        glfwShowWindow(window_);

        glfwSetWindowUserPointer(window_, app);
        glfwSetKeyCallback(window_,&key_callback);
    }

    // destroy the lib, close the window
    ~graphics()
    {
        if(window_)
        {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }

        glfwTerminate();
    }

    bool running() { return !glfwWindowShouldClose( window_ ); }

    void setMVP(const glm::mat4 &mvp) const
    {
        glUniformMatrix4fv(mvp_handle_, 1, GL_FALSE, &mvp[0][0]);
    }

    void setTexOffset(const glm::mat4 &mvp) const
    {
        //glUniformMatrix3fv(tex_mvp_, 1, GL_FALSE, &offset[0][0]);
        glUniformMatrix4fv(tex_mvp_, 1, GL_FALSE, &mvp[0][0]);
    }

    void swap()
    {
        glfwSwapBuffers(window_);
    }

    // clear the display buffer
    void clear()
    {
        int w,h;
        float wscale,hscale;
        glfwGetWindowSize(window_, &w, &h);
        // TODO that makes no sense
        //glViewport(0,0, w*2, h*2);
        //
        //-> This is about fractional scaling
        glfwGetWindowContentScale(window_, &wscale, &hscale);
        glViewport(0,0, w*wscale, h*hscale);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void pollEvents()
    {
        glfwPollEvents();
    }

}; // struct graphics

////////////////////////////////////////
/// struct world
////////////////////////////////////////


GLuint load_texture(const char *path)
{
    GLuint id{0};
    // The mode is needed since the imagedata got saved by gimp
    // without alpha channel once. In that case I have only 3 channels
    // which messes up the import if that is const.
    GLuint mode{GL_RGBA};
    // stbi loads image with first pixel top-left.
    // OpenGL expects first pixel to be bottom-left in UV coords.
    // With this function the pixel data comes as expected
    // and the vertex data may be written like tutorials would
    // write them ;-)
    stbi_set_flip_vertically_on_load(true);

    // load the image
    int x=0,y=0,nrChannels=0;
    auto * mem = stbi_load(path, &x, &y, &nrChannels, 0);
    printf("image=%s,x=%d,y=%d,nrOfCh=%d\n", path, x, y, nrChannels);

    assert(x != 0 && "x is 0");
    assert(y != 0 && "y is 0");
    assert(mem != NULL && "loading image failed");

    glGenTextures(1, &id);

    if(nrChannels == 3)
    {
        mode = GL_RGB;
    }

    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, mode, x, y, 0, mode, GL_UNSIGNED_BYTE, mem);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(mem);

    return id;
}

GLuint load_vao_for_texture()
{
    GLuint vao{0};
    const GLfloat vertices[] =
    {
        // position, texture coord
        -1.0f, -1.0f, 0.0f, 0.0f, // bottom left
        1.0f, -1.0f, 1.0f, 0.0f,  // bottom right
        -1.0f,  1.0f, 0.0f, 1.0f, // top left
        1.0f, -1.0f, 1.0f, 0.0f,  // bottom right
        1.0f,  1.0f, 1.0f, 1.0f,  // top right
        -1.0f,  1.0f, 0.0f, 1.0f  // top left
    };

    // sammelt die erstellten vbo
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // the position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void *)0);
    glEnableVertexAttribArray(0);

    // the texture
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    return vao;
}


struct world
{
    vec2 max{10, 10};

    world() {}

    void init()
    {
        // Load the image into a texture
        texture_id_ = load_texture("imgs/world.png");

        // Load the vertices
        vao_ = load_vao_for_texture();
    }

    void render(const graphics &g)
    {
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glBindVertexArray(vao_);

        // identity matrix
        glm::mat4 translate = glm::mat4(1.0);
        g.setMVP(translate);
        g.setTexOffset(glm::mat4(1.0));

        glDrawArrays(GL_TRIANGLES, 0,6);
    }

private:
    GLuint vao_{0};
    GLuint texture_id_{0};
};

struct snake : public isnake
{
    snake(const vec2 &start) : head_(start) {}

    bool got_section() const noexcept
    {
            auto begin = tail_.end() - snake_len_;
            auto end = tail_.end();

            for(auto b = begin; b < end; ++b)
            {
                if(*b == head())
                {
                    return true;
                }
            }
            return false;
    }

    void grow()
    {
        snake_len_ += 1;
    }

    const vec2 & head() const
    {
        return head_;
    }

    void up() override
    {
        goDir(direction::up, direction::down);
    }

    void down() override
    {
        goDir(direction::down, direction::up);
    }

    void left() override
    {
        goDir(direction::left, direction::right);
    }

    void right() override
    {
        goDir(direction::right, direction::left);
    }

    void move(const world &w)
    {
        tail_.emplace_back(head_);

        if(tail_.size() > 64)
        {
            tail_.erase(tail_.begin(), tail_.end() - snake_len_);
        }

        switch(current_dir_)
        {
            case direction::up:
                head_.second = (head_.second == w.max.second - 1) ? 0 : head_.second + 1;
                break;
            case direction::down:
                head_.second = (head_.second == 0) ? w.max.second - 1 : head_.second - 1;
                break;
            case direction::left:
                head_.first = (head_.first == 0) ? w.max.first - 1 : head_.first - 1;
                break;
            case direction::right:
                head_.first = (head_.first == w.max.first - 1) ? 0 : head_.first + 1;
                break;
        };

        last_dir_ = current_dir_;
    }

    void init()
    {
        // Load the image into a texture
        texture_id_ = load_texture("imgs/ball.png");

        // Load the vertices
        vao_ = load_vao_for_texture();
    }

    void render(const graphics &g, const world &w)
    {
        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glBindVertexArray(vao_);

        // so that PNG alphachannel works
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        drawSection(head_, g, w);

        for (auto it = tail_.cend() - std::min(snake_len_, tail_.size());
                it < tail_.cend(); ++it)
        {
            drawSection(*it,g,w);
        }

        glDisable(GL_BLEND);
    }

    void drawSection(const vec2 &pos,const graphics &g, const world &w)
    {
        // get the current position for drawing
        const auto xpos = -1.0 + (pos.first+1) * 2.0 / (w.max.first + 1);
        const auto ypos = -1.0 + (pos.second+1) * 2.0 / (w.max.second + 1);

        glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(xpos,ypos,0));

        glm::mat4 scale = glm::scale(glm::vec3(0.1,0.1,0.1));

        g.setMVP(translate * scale);

        // is on position 0, pixel is 8x8
        // NOTE This makes not much sense right now because I load the same 'tilemap/image' twice.
        // NOTE If the image is shared as a real tilemap and the number of tiles is known, this magic
        // NOTE number becomes logical (1/8 in my case).
        g.setTexOffset(glm::scale(glm::vec3(0.125,0.125,0)));

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

private:
    // the current len
    size_t snake_len_{1};

    vec2 head_;

    std::vector<vec2> tail_;

    enum class direction { up, left, right, down } current_dir_{direction::up};
    direction last_dir_{direction::up};

    void goDir(const direction &dir, const direction &ignore)
    {
        if(last_dir_ == dir || last_dir_ == ignore )
        {
            return;
        }

        current_dir_ = dir;
    }

    GLuint vao_{0};
    GLuint texture_id_{0};
};


////////////////////////////////////////
/// items
////////////////////////////////////////

struct item
{
    item(const vec2 &start) : pos_(start) {}

    const vec2 & pos() const
    {
        return pos_;
    }

    void init()
    {
        // load the image into a texture
        texture_id_ = load_texture("imgs/ball.png");

        // Load the vertices
        vao_ = load_vao_for_texture();
    }

    void render(const graphics &g, const world &w)
    {
        assert(item::vao_ != 0 && "vao is 0");
        assert(item::texture_id_ != 0 && "texture_id_ is 0");

        if(hide_ == true)
        {
            return;
        }

        // so that PNG alphachannel works
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindTexture(GL_TEXTURE_2D, texture_id_);
        glBindVertexArray(vao_);

        // get the current position for drawing
        const auto xpos = -1.0 + (pos_.first+1) * 2.0 / (w.max.first + 1);
        const auto ypos = -1.0 + (pos_.second+1) * 2.0 / (w.max.second + 1);

        glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(xpos,ypos,0));

        glm::mat4 scale = glm::scale(glm::vec3(0.1,0.1,0.1));

        g.setMVP(translate * scale);

        // is on position 1, pixel is 8x8
        g.setTexOffset(glm::translate(glm::mat4(1.0), glm::vec3(0.125,0,0)) * glm::scale(glm::vec3(0.125,0.125,0)));

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisable(GL_BLEND);
    }

    void hide()
    {
        hide_ = true;
    }

    bool is_hidden() const noexcept
    {
        return hide_;
    }

    void reset(const double &time)
    {
        pos_.first = (int)time % 10;
        pos_.second = (int)(time * 1000) % 10;
        hide_ = false;
    }

private:
    vec2 pos_;

    bool hide_{false};

    // do that multiple times
    GLuint vao_;
    GLuint texture_id_;
};

////////////////////////////////////////
/// main
////////////////////////////////////////

int main()
{

    app app;

    graphics g(&app);
    world w;
    snake s({5,5});

    item i({6,7});

    app.g = &g;
    app.w = &w;
    app.s = &s;
    app.ready = true;

    w.init();
    s.init();
    i.init();

    double last_update = -1;
    double last_item = -1;
    int grown = 0;

    double move_time_ = 0.5;

    while(g.running())
    {
        // clear the screen
        g.clear();

        double secs = glfwGetTime();
        if((secs - last_update) > move_time_)
        {
            last_update = secs;
            s.move(w);

            // got own section
            if(s.got_section())
            {
                printf("ha, loser\n");
                return 1;
            }

            // got item
            if(s.head() == i.pos())
            {
                i.hide();
                s.grow();
                grown += 1;

                // increase after 3 items
                if((grown % 4) == 0)
                {
                    move_time_ -= 0.1;
                }
            }
        }

        if(move_time_ <= 0.001)
        {
            printf("jay you made it\n");
            return 0;
        }

        if(secs - last_item > 5.0 && i.is_hidden())
        {
            i.reset(secs);
            last_item = secs;
        }

        // draw the world
        w.render(g);

        // draw the snake
        s.render(g,w);

        // draw the item
        i.render(g,w);

        // swap the buffer and wait for another event
        g.swap();
        g.pollEvents();
    }

    return 0;
}
