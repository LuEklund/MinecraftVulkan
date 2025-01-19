//
// Created by jasper on 18-1-2025.
//

#ifndef SHADERCOMPILER_HPP
#define SHADERCOMPILER_HPP
#include <vector>


enum ShaderType
{
    Vertex,
    Fragment,
    Compute
};

bool CompileShader(const char* filePath, const ShaderType type, std::vector<uint32_t>& code);




#endif //SHADERCOMPILER_HPP
