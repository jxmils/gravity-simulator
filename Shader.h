#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glad/glad.h>

class Shader {
public:
    GLuint ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use() const;

    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
};

#endif // SHADER_H