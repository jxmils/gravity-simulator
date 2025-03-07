#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) : ID(0) {
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;
    
    // Ensure ifstream objects can throw exceptions
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        // Open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        
        // Read file contents into streams
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        
        // Close files
        vShaderFile.close();
        fShaderFile.close();
        
        // Convert streams into strings
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        if (!vShaderFile.is_open()) {
            std::cerr << "Could not open vertex shader file: " << vertexPath << std::endl;
        }
        if (!fShaderFile.is_open()) {
            std::cerr << "Could not open fragment shader file: " << fragmentPath << std::endl;
        }
        throw;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    std::cout << "Vertex Shader Code:\n" << vertexCode << std::endl;
    std::cout << "Fragment Shader Code:\n" << fragmentCode << std::endl;

    GLuint vertex = 0, fragment = 0;
    int success;
    char infoLog[512];

    try {
        // Compile vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        if (vertex == 0) {
            throw std::runtime_error("Failed to create vertex shader");
        }
        
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            throw std::runtime_error(std::string("Vertex shader compilation failed:\n") + infoLog);
        }
        std::cout << "Vertex shader compiled successfully!" << std::endl;

        // Compile fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        if (fragment == 0) {
            throw std::runtime_error("Failed to create fragment shader");
        }
        
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            throw std::runtime_error(std::string("Fragment shader compilation failed:\n") + infoLog);
        }
        std::cout << "Fragment shader compiled successfully!" << std::endl;

        // Create and link shader program
        ID = glCreateProgram();
        if (ID == 0) {
            throw std::runtime_error("Failed to create shader program");
        }
        
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            throw std::runtime_error(std::string("Shader program linking failed:\n") + infoLog);
        }
        std::cout << "Shader program linked successfully! Program ID: " << ID << std::endl;
    }
    catch (const std::exception& e) {
        // Clean up on error
        if (vertex != 0) {
            glDeleteShader(vertex);
        }
        if (fragment != 0) {
            glDeleteShader(fragment);
        }
        if (ID != 0) {
            glDeleteProgram(ID);
            ID = 0;
        }
        std::cerr << "ERROR::SHADER::INITIALIZATION_FAILED: " << e.what() << std::endl;
        throw;
    }

    // Clean up shaders after successful linking
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Verify program is valid
    glValidateProgram(ID);
    glGetProgramiv(ID, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cerr << "WARNING::SHADER::PROGRAM_VALIDATION_FAILED:\n" << infoLog << std::endl;
    }
}

Shader::~Shader() {
    if (ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
}

void Shader::use() const {
    if (ID != 0) {
        glUseProgram(ID);
    } else {
        std::cerr << "ERROR::SHADER::INVALID_PROGRAM_ID" << std::endl;
    }
}

void Shader::setFloat(const std::string& name, float value) const {
    if (ID == 0) {
        std::cerr << "ERROR::SHADER::INVALID_PROGRAM_ID" << std::endl;
        return;
    }
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform1f(location, value);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    if (ID == 0) {
        std::cerr << "ERROR::SHADER::INVALID_PROGRAM_ID" << std::endl;
        return;
    }
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform3f(location, x, y, z);
}