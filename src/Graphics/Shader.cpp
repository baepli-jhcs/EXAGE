#include <array>
#include <fstream>
#include <memory>

#include "exage/Graphics/Shader.h"

#include "exage/Graphics/Error.h"
#include "fmt/core.h"
#include "shaderc/shaderc.hpp"

namespace exage::Graphics
{
    namespace
    {

        class ShaderIncluder : public shaderc::CompileOptions::IncluderInterface
        {
            auto GetInclude(const char* requestedSource,
                            shaderc_include_type type,
                            const char* requestingSource,
                            size_t includeDepth) -> shaderc_include_result* override
            {
                std::string msg = std::string(requestingSource);
                msg += std::to_string(type);
                msg += static_cast<char>(includeDepth);

                const std::string requestedSourcePath =
                    getFilePath(requestedSource, requestingSource);
                const std::string contents = readFile(requestedSourcePath);

                auto* container = new std::array<std::string, 2>;
                (*container)[0] = requestedSourcePath;
                (*container)[1] = contents;

                auto* data = new shaderc_include_result;

                data->user_data = container;

                data->source_name = (*container)[0].data();
                data->source_name_length = (*container)[0].size();

                data->content = (*container)[1].data();
                data->content_length = (*container)[1].size();

                return data;
            }
            void ReleaseInclude(shaderc_include_result* data) override
            {
                auto* container = static_cast<std::array<std::string, 2>*>(data->user_data);
                delete container;
                delete data;
            }

            static auto getFilePath(const std::filesystem::path& requestedSource,
                                    const std::string& requestingSource) -> std::string
            {
                std::filesystem::path path(requestingSource);
                path.remove_filename();
                path /= requestedSource;
                return path.string();
            }

            static auto readFile(const std::string& filepath) -> std::string
            {
                std::string sourceCode;
                std::ifstream in(filepath, std::ios::in | std::ios::binary);
                if (in)
                {
                    in.seekg(0, std::ios::end);
                    size_t size = in.tellg();
                    if (size > 0)
                    {
                        sourceCode.resize(size);
                        in.seekg(0, std::ios::beg);
                        in.read(sourceCode.data(), static_cast<std::streamsize>(size));
                    }
                    else
                    {
                        fmt::print("Could not read from file '{0}'", filepath);
                    }
                }
                else
                {
                    fmt::print("Could not open file '{0}'", filepath);
                }
                return sourceCode;
            }
        };
    }  // namespace

    auto compileShaderToIR(const std::filesystem::path& path, Shader::Stage stage) noexcept
        -> tl::expected<std::vector<uint32_t>, Error>
    {
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;

        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetIncluder(std::unique_ptr<shaderc::CompileOptions::IncluderInterface>(
            new (std::nothrow) ShaderIncluder));

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
            return tl::make_unexpected(Errors::FileNotFound {path});
        }

        std::ifstream shaderFile(path);
        if (!shaderFile.is_open())
        {
            return tl::make_unexpected(Errors::FileNotFound {path});
        }

        std::string shaderSource((std::istreambuf_iterator<char>(shaderFile)),
                                 std::istreambuf_iterator<char>());

        shaderc::SpvCompilationResult module =
            compiler.CompileGlslToSpv(shaderSource, shaderKind, path.string().c_str(), options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            std::string msg = module.GetErrorMessage();
            return tl::make_unexpected(Errors::ShaderCompilationFailed {msg});
        }

        return std::vector<uint32_t>(module.cbegin(), module.cend());
    }

}  // namespace exage::Graphics
