#include "ShaderCompiler.hpp"

#include <fstream>
#include <iostream>
#include <shaderc/shaderc.hpp>

shaderc_shader_kind GetShaderType(const ShaderType type) {
    switch (type) {
        case Vertex:
            return shaderc_glsl_vertex_shader;
        case Fragment:
            return shaderc_glsl_fragment_shader;
        case Compute:
            return shaderc_glsl_compute_shader;
    }
    throw std::invalid_argument("Unsupported shader type");
}


bool CompileShader(const char *filePath, const ShaderType type, std::vector<uint32_t> &code) {
    shaderc::Compiler compiler;

    std::string source;
    std::string line;
    std::ifstream file(filePath);
    if (file.is_open()) {
        while (getline(file, line)) {
            source += line + "\n";
        }
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filePath << std::endl;
        return false;
    }

    std::cout << "Compiling shader: " << filePath << std::endl;
    std::cout << "Source: " << source << std::endl;

    shaderc::CompileOptions options;
    shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, GetShaderType(type), filePath, options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << "Failed to compile shader: " << result.GetErrorMessage() << std::endl;
        return false;
    }

    code.assign(result.cbegin(), result.cend());
    std::cout << "Compiled shader with " << result.GetNumWarnings() << " warnings" << std::endl;

    std::cout << "spv: " << code.size() << std::endl;


    return true;


}
