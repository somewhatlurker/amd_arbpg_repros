#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

const char test_vp[] = "!!ARBvp1.0\n"
                       //"OPTION NV_vertex_program3;\n"
                       "MOV result.position, vertex.attrib[0];\n"
                       "MOV result.texcoord[0], vertex.attrib[1];\n"
                       //"TEMP a; TEX a, vertex.attrib[1], texture[16], 2D;\n"
                       "END";

const char test_fp_fmt[] = "!!ARBfp1.0\n"
                           "TEX result.color, fragment.texcoord[0], texture[%d], 2D;\n"
                           "END";

char test_fp[sizeof(test_fp_fmt)]; // fill using snprintf from above format,
                                   // length is same because %d takes same
                                   // number of chars as two-digit numbers


// Create and compile fragment program from a string, logging compile errors
// to stderr. Program will be left bound after completion.
// Returns true on success, false on failure.
//     id: OUTPUT, GL program number used
//     fp: INPUT, fragment program string (null terminated)
bool create_fp(GLuint *id, const char *fp) {
    // not sure where proper docs are for correct creation process, so based on
    // Nvidia GDC presentation: https://www.nvidia.com/docs/IO/8227/GDC2003_OGL_ARBVertexProgram.pdf
    glGenProgramsARB(1, id);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, *id);
    glProgramStringARB(
        GL_FRAGMENT_PROGRAM_ARB,
        GL_PROGRAM_FORMAT_ASCII_ARB,
        strlen(fp),
        fp
    );
    
    if (glGetError() == GL_INVALID_OPERATION) {
        GLint err_pos;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &err_pos);
        
        const GLubyte *err_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        
        if (err_pos != -1) {
            fprintf(
                stderr,
                "Fragment program compilation error at pos %d:\n  %s\n",
                err_pos, err_string
            );
        }
        
        return false;
    }
    
    return true;
}


// Create and compile vertex program from a string, logging compile errors
// to stderr. Program will be left bound after completion.
// Returns true on success, false on failure.
//     id: OUTPUT, GL program number used
//     fp: INPUT, vertex program string (null terminated)
bool create_vp(GLuint *id, const char *fp) {
    // not sure where proper docs are for correct creation process, so based on
    // Nvidia GDC presentation: https://www.nvidia.com/docs/IO/8227/GDC2003_OGL_ARBVertexProgram.pdf
    glGenProgramsARB(1, id);
    glBindProgramARB(GL_VERTEX_PROGRAM_ARB, *id);
    glProgramStringARB(
        GL_VERTEX_PROGRAM_ARB,
        GL_PROGRAM_FORMAT_ASCII_ARB,
        strlen(fp),
        fp
    );
    
    if (glGetError() == GL_INVALID_OPERATION) {
        GLint err_pos;
        glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &err_pos);
        
        const GLubyte *err_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        
        if (err_pos != -1) {
            fprintf(
                stderr,
                "Vertex program compilation error at pos %d:\n  %s\n",
                err_pos, err_string
            );
        }
        
        return false;
    }
    
    return true;
}


// Delete ARB program with given id.
//     id: INPUT, GL program number
void delete_prog(GLuint id) {
    glDeleteProgramsARB(1, &id);
}


void print_extensions() {
    GLint count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &count);

    for (int i = 0; i < count; ++i)
        printf("  %s\n", glGetStringi(GL_EXTENSIONS, i));
}


void glfw_err_cb(int error, const char *description) {
    fprintf(stderr, "GLFW error: %i (%s)\n", error, description);
}

void glfw_close_cb(GLFWwindow *window) {
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(0);
}


// bottom-right half-screen triangle
const float tri_verts[] = {
//  x,     y,     u,     v
    1.0f,  1.0f,  1.0f,  0.0f,  // top-right corner
   -1.0f, -1.0f,  0.0f,  1.0f,  // bottom-left corner
    1.0f, -1.0f,  1.0f,  1.0f,  // bottom-right corner
};

// red->green texture (top->bottom)
const float rg_tex[] = {
//  r,     g,     b
    1.0f,  0.0f,  0.0f,
    0.0f,  1.0f,  0.0f,
};


int main() {
    // init window and context
    if (!glfwInit()) {
        fprintf(stderr, "Error: GLFW initialisation failed\n");
        return 1;
    }
    
    glfwSetErrorCallback(glfw_err_cb);
        
    unsigned windowWidth = 256;
    unsigned windowHeight = 256;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "test", NULL, NULL);
    if (!window) {
        fprintf(stderr, "GLFW window or context creation failed\n");
        return 1;
    }
    
    glfwSetWindowCloseCallback(window, glfw_close_cb);
        
    glfwMakeContextCurrent(window);
    
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    glfwSwapInterval(0); // no vsync
    
    glClearColor(0, 0, 1, 1);
    
    // print_extensions();
    
    
    // vbo for triangle
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tri_verts), tri_verts, GL_STATIC_DRAW);
    
    glVertexAttribPointerARB(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    glVertexAttribPointerARB(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArrayARB(0);
    glEnableVertexAttribArrayARB(1);
    
    
    // create red->green texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        1, sizeof(rg_tex)/sizeof(float)/3, 0,
         GL_RGB, GL_FLOAT, rg_tex
    );
    
    
    // create vertex program (constant)
    GLuint vp;
    if (!create_vp(&vp, test_vp)) {
        return 1;
    }
    
    glEnable(GL_VERTEX_PROGRAM_ARB);
    
    
    // create fragment program (for maximum valid tex image unit)
    GLint max_tex_img_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS_ARB, &max_tex_img_units);
    // max_tex_img_units = 16; // uncomment for working sample
    fprintf(stderr, "Info: MAX_TEXTURE_IMAGE_UNITS_ARB = %d\n", max_tex_img_units);
    
    snprintf(test_fp, sizeof(test_fp), test_fp_fmt, max_tex_img_units - 1);
    GLuint fp;
    if (!create_fp(&fp, test_fp)) {
        fprintf(stderr, "Texture image unit %d not allowed.\n", max_tex_img_units - 1);
        fprintf(stderr, "Finding max unit really allowed by driver...\n");
        
        for (int i = max_tex_img_units - 2; i >= 0; --i) {
            fprintf(stderr, "%d...", i);
            snprintf(test_fp, sizeof(test_fp), test_fp_fmt, i);
            GLuint tmp_fp;
            if (create_fp(&tmp_fp, test_fp)) {
                fprintf(stderr, "Maximum (no compilation error): %d\n", i);
                delete_prog(tmp_fp);
                break;
            } else {
                delete_prog(tmp_fp);
            }
        }
        
        // re-bind original fp
        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fp);
    }
    
    // bind texture to last texture unit
    glActiveTexture(GL_TEXTURE0 + max_tex_img_units - 1);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    
    while (true) {
        glfwPollEvents();
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        // update viewport
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
    }
    
    return 0;
}
