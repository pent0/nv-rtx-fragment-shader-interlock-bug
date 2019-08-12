#include "glad.h"
#include "GLFW/glfw3.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#pragma comment(lib, "glfw3.lib")

struct game_state {
    // Framebuffer
    GLuint game_framebuffer;
    GLuint game_texture_color;
    GLuint game_renderbuffer_depth;

    // Shader
    GLuint game_fill_program;
    GLuint game_framebuffer_render_program;

    // Uniforms
    GLuint game_fill_program_uscreen_location;
    GLuint game_fill_program_utexture_location;

    // Buffers
    GLuint game_vertex_buffer;
    GLuint game_clear_screen_vertex_buffer;
    GLuint game_shading_circle_vertex_buffer;

    GLuint game_vertex_array;
    GLuint game_clear_screen_vertex_array;
    GLuint game_shading_circle_vertex_array;
    GLuint game_vertex_indicies;
    GLuint game_shading_circle_vertex_indices;

    GLuint quad_vertex_buffer;
    GLuint quad_vertex_array;

    int width;
    int height;
};

struct tv_screen_vertex_buffer {
    float position[2];
    float coords[2];
    float colors[4];
};

const tv_screen_vertex_buffer buffer_data[] = {
    { {835.40411f,  418.42172f }, {128.0f, 0.0f}, {0.45882f, 0.45882f, 0.75088f, 1.0f}},    // top right
    { {914.32806f,  436.64276f }, {128.0f, 128.0f}, {0.45882f, 0.45882f, 0.75088f, 1.0f}},  // bottom right
    { {822.13202f,  475.90955f }, {0.0f, 0.0f}, {0.45882f, 0.45882f, 0.75088f, 1.0f}},      // top left
    { {901.05597f,  494.13058f }, {0.0f, 128.0f}, {0.45882f, 0.45882f, 0.75088f, 1.0f}}     // bottom left
};

const tv_screen_vertex_buffer clear_screen_buffer_data[] = {
    { {0.0f,  0.0f }, {128.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},    // top right
    { {0.0f, 544.0f }, {128.0f, 128.0f}, {0.0f, 0.0f, 0.0f, 1.0f}},  // bottom right
    { {960.0f,  0.0f }, {0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}},      // top left
    { {960.0f,  544.0f }, {0.0f, 128.0f}, {0.0f, 0.0f, 0.0f, 1.0f}}     // bottom left
};

const uint32_t buffer_indices[] = {
    0, 1, 2, 3
};

const float quad_vertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

game_state state;

#pragma region UTILITIES

int read_text(std::string& dest, const std::string& path) {
    std::ifstream file_stream(path);

    if (file_stream.fail()) {
        return -1;
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();

    dest = std::move(buffer.str());

    return 0;
}

int read_binary(std::string& data, const std::string& path) {
    std::ifstream file_stream(path, std::ios_base::ate | std::ios_base::binary);
    
    if (file_stream.fail()) {
        return -1;
    }

    auto size = file_stream.tellg();
    data.resize(size);

    file_stream.seekg(0, std::ios_base::beg);
    file_stream.read(&data[0], size);

    return 0;
}

bool shader_compile_success(GLuint obj, GLchar *fail_msg, GLuint msg_max_size) {
    int status = GL_FALSE;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        glGetShaderInfoLog(obj, msg_max_size, nullptr, fail_msg);
    }

    return status == GL_TRUE;
}

bool program_link_success(GLuint obj, GLchar* fail_msg, GLuint msg_max_size) {
    int status = GL_FALSE;
    glGetProgramiv(obj, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        glGetProgramInfoLog(obj, msg_max_size, nullptr, fail_msg);
    }

    return status == GL_TRUE;
}

#pragma endregion

#pragma region GL_PREPARATION

#define CHECK_SHADER_STATUS(name, obj, msg, size)                     \
    if (shader_compile_success(obj, msg, size) == GL_FALSE) {         \
        std::cout << "Error compiling " << name << "\n";              \
        std::cout << msg << "\n";                                     \
        return -1;                                                    \
    }

#define CHECK_PROGRAM_STATUS(name, obj, msg, size)                    \
    if (program_link_success(obj, msg, size) == GL_FALSE) {           \
        std::cout << "Error linking " << name << "\n";                \
        std::cout << msg << "\n";                                     \
        return -1;                                                    \
    }

int gen_tv_buffers(game_state& state) {
    glGenBuffers(1, &state.game_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, state.game_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer_data), buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &state.game_clear_screen_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, state.game_clear_screen_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(clear_screen_buffer_data), clear_screen_buffer_data, GL_STATIC_DRAW);

    glGenVertexArrays(1, &state.game_vertex_array);
    glGenVertexArrays(1, &state.game_clear_screen_vertex_array);

    glBindVertexArray(state.game_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, state.game_vertex_buffer);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (GLvoid*)(4 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glGenBuffers(1, &state.game_vertex_indicies);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.game_vertex_indicies);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(buffer_indices), buffer_indices, GL_STATIC_DRAW);

    glBindVertexArray(state.game_clear_screen_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, state.game_clear_screen_vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.game_vertex_indicies);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, 8 * sizeof(float), (GLvoid*)(4 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &state.quad_vertex_array);
    glGenBuffers(1, &state.quad_vertex_buffer);
    glBindVertexArray(state.quad_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, state.quad_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glGenVertexArrays(1, &state.game_shading_circle_vertex_array);
    glGenBuffers(1, &state.game_shading_circle_vertex_buffer);
    glBindVertexArray(state.game_shading_circle_vertex_array);
    glBindBuffer(GL_ARRAY_BUFFER, state.game_shading_circle_vertex_buffer);

    std::string buf_data;
    if (read_binary(buf_data, "shading_circle.bin") != 0) {
        return -1;
    }

    glBufferData(GL_ARRAY_BUFFER, buf_data.size(), &buf_data[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 16, (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 16, (void*)(3 * sizeof(float)));

    GLushort shading_buf_indicies[480];

    for (std::uint32_t i = 0; i < 480; i++) {
        shading_buf_indicies[i] = i;
    }

    glGenBuffers(1, &state.game_shading_circle_vertex_indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.game_shading_circle_vertex_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shading_buf_indicies), shading_buf_indicies, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return 0;
}

int gen_shader_program(game_state& state) {
    std::string fill_frag_shader_text;
    std::string display_vert_shader_text;
    std::string framebuffer_vert_shader_text;
    std::string framebuffer_frag_shader_text;

    if (read_text(display_vert_shader_text, "display.vert") == -1 ||
        read_text(fill_frag_shader_text, "fill.frag") == -1 ||
        read_text(framebuffer_vert_shader_text, "framebuffer_render.vert") == -1 ||
        read_text(framebuffer_frag_shader_text, "framebuffer_render.frag") == -1) {
        return -1;
    }

    const GLchar* vert_shader_sources[] = { display_vert_shader_text.c_str() };
    const GLchar* fill_frag_shader_sources[] = { fill_frag_shader_text.c_str() };

    GLchar message[512];

    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, vert_shader_sources, nullptr);
    glCompileShader(vert_shader);

    CHECK_SHADER_STATUS("display.vert", vert_shader, message, 512);

    GLuint fill_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fill_shader, 1, fill_frag_shader_sources, nullptr);
    glCompileShader(fill_shader);

    CHECK_SHADER_STATUS("fill.frag", fill_shader, message, 512);

    state.game_fill_program = glCreateProgram();
    glAttachShader(state.game_fill_program, vert_shader);
    glAttachShader(state.game_fill_program, fill_shader);
    glLinkProgram(state.game_fill_program);

    CHECK_PROGRAM_STATUS("fill program", state.game_fill_program, message, 512);

    glDeleteShader(vert_shader);
    glDeleteShader(fill_shader);

    vert_shader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar* framebuffer_vert_shader_sources[] = { framebuffer_vert_shader_text.c_str() };
    glShaderSource(vert_shader, 1, framebuffer_vert_shader_sources, nullptr);
    glCompileShader(vert_shader);

    CHECK_SHADER_STATUS("framebuffer_render.vert", vert_shader, message, 512);

    GLuint frag_framebuffer_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar* framebuffer_frag_shader_sources[] = { framebuffer_frag_shader_text.c_str() };
    glShaderSource(frag_framebuffer_shader, 1, framebuffer_frag_shader_sources, nullptr);
    glCompileShader(frag_framebuffer_shader);

    CHECK_SHADER_STATUS("framebuffer_render.frag", vert_shader, message, 512);

    state.game_framebuffer_render_program = glCreateProgram();
    glAttachShader(state.game_framebuffer_render_program, vert_shader);
    glAttachShader(state.game_framebuffer_render_program, frag_framebuffer_shader);
    glLinkProgram(state.game_framebuffer_render_program);

    CHECK_PROGRAM_STATUS("framebuffer renderer program", state.game_framebuffer_render_program, message, 512);

    glDeleteShader(vert_shader);
    glDeleteShader(frag_framebuffer_shader);

    return 0;
}

int gen_framebuffer(game_state &state) {
    glGenFramebuffers(1, &state.game_framebuffer);
    glGenRenderbuffers(1, &state.game_renderbuffer_depth);

    glGenTextures(1, &state.game_texture_color);
    glBindTexture(GL_TEXTURE_2D, state.game_texture_color);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 960, 544, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, state.game_renderbuffer_depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 960, 544);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, state.game_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state.game_texture_color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, state.game_renderbuffer_depth);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }

    glClearColor(0.5f, 0.3f, 0.6f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return -1;
}

int setup(game_state& state) {
    if (gen_shader_program(state) != 0) {
        return -1;
    }

    if (gen_framebuffer(state) != 0) {
        return -1;
    }

    if (gen_tv_buffers(state) != 0) {
        return -1;
    }

    state.game_fill_program_uscreen_location = glGetUniformLocation(state.game_fill_program, "uScreen");
    state.game_fill_program_utexture_location = glGetUniformLocation(state.game_fill_program, "uTexture");

    return 0;
}

#pragma endregion

void changeSize(GLFWwindow *win, int w, int h) {
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    glViewport(0, 0, w, h);

    state.width = w;
    state.height = h;
}

void renderScene(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, state.game_framebuffer);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glBindVertexArray(state.game_clear_screen_vertex_array);

    // Bind framebuffer color attachment 0
    glBindImageTexture(0, state.game_texture_color, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

    // Clear screen
    glUseProgram(state.game_fill_program);
    glUniform2f(state.game_fill_program_uscreen_location, state.width, state.height);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);

    // Draw box
    glBindVertexArray(state.game_vertex_array);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);

    // Draw shading
    glBindVertexArray(state.game_shading_circle_vertex_array);
    glDrawElements(GL_TRIANGLE_STRIP, 480, GL_UNSIGNED_SHORT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(state.game_framebuffer_render_program);
    glBindVertexArray(state.quad_vertex_array);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state.game_texture_color);     // use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int main(int argc, char **argv) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(960, 544, "Repro", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    changeSize(window, 960, 544);
    glfwSetFramebufferSizeCallback(window, changeSize);

    int result = setup(state);

    while (!glfwWindowShouldClose(window)) {
        renderScene();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

	return 0;
}