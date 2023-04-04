#include <fstream>

#include "exage/Graphics/Shader.h"

#include "shaderc/shaderc.hpp"

namespace exage::Graphics
{
    auto compileShaderToIR(const std::filesystem::path& path,
                           Shader::Stage stage,
                           bool cache,
                           bool forceRecompile) noexcept
        -> tl::expected<std::vector<uint32_t>, Error>
    {
        if (!forceRecompile)
        {
            std::filesystem::path saveName = path.parent_path() / "bin" / path.stem();
            saveName.replace_extension(".spv");

            if (std::filesystem::exists(saveName))
            {
                std::ifstream file(saveName, std::ios::binary | std::ios::ate);
                if (file.is_open())
                {
                    std::vector<uint32_t> result;
                    std::streamsize size = file.tellg();
                    result.resize(size / sizeof(uint32_t));
                    file.seekg(0, std::ios::beg);
                    file.read(reinterpret_cast<char*>(result.data()), size);
                    return result;
                }
            }
        }

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

#ifdef EXAGE_DEBUG
        options.SetGenerateDebugInfo();
#endif
        shaderc_shader_kind shaderKind;
        switch (stage)
        {
            case Shader::Stage::eVertex:
                shaderKind = shaderc_glsl_vertex_shader;
                break;
            case Shader::Stage::eTessellationControl:
                shaderKind = shaderc_glsl_tess_control_shader;
                break;
            case Shader::Stage::eTessellationEvaluation:
                shaderKind = shaderc_glsl_tess_evaluation_shader;
                break;
            case Shader::Stage::eFragment:
                shaderKind = shaderc_glsl_fragment_shader;
                break;
            case Shader::Stage::eCompute:
                shaderKind = shaderc_glsl_compute_shader;
                break;
            default:
                EXAGE_UNREACHABLE;
        }

        if (!std::filesystem::exists(path))
        {
            return tl::make_unexpected(FileError::eFileNotFound);
        }

        std::ifstream shaderFile(path);
        if (!shaderFile.is_open())
        {
            return tl::make_unexpected(FileError::eFileNotReadable);
        }

        std::string shaderSource((std::istreambuf_iterator<char>(shaderFile)),
                                 std::istreambuf_iterator<char>());

        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
            shaderSource, shaderc_glsl_fragment_shader, path.filename().string().c_str(), options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            return tl::make_unexpected(Errors::ShaderCompileFailed {module.GetErrorMessage()});
        }

        if (cache)
        {
            std::filesystem::path saveName = path.parent_path() / "bin" / path.stem();
            saveName.replace_extension(".spv");
            std::filesystem::create_directories(saveName.parent_path());
            std::ofstream file(saveName, std::ios::binary);
            if (file.is_open())
            {
                file.write(reinterpret_cast<const char*>(module.cbegin()),
                           module.cend() - module.cbegin());
            }
        }
        return std::vector<uint32_t>(module.cbegin(), module.cend());
    }
}  // namespace exage::Graphics
