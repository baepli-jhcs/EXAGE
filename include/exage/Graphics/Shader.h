#pragma once

#include <filesystem>
#include <span>

#include "exage/Core/Core.h"
#include "exage/Graphics/Context.h"

namespace exage::Graphics
{
    class EXAGE_EXPORT Shader
    {
      public:
        enum class Stage;
        Shader() noexcept = default;
        virtual ~Shader() = default;

        EXAGE_DEFAULT_COPY(Shader);
        EXAGE_DEFAULT_MOVE(Shader);

        [[nodiscard]] auto getStage() const noexcept -> Stage { return _stage; }

        EXAGE_BASE_API(API, Shader);

        enum class Stage
        {
            eVertex,
            eTessellationControl,
            eTessellationEvaluation,
            eFragment,
            eCompute,
        };

      protected:
        Stage _stage = Stage::eVertex;

        Shader(Stage stage) noexcept
            : _stage(stage)
        {
        }
    };

    EXAGE_EXPORT auto compileShaderToIR(const std::filesystem::path& path,
                                        Shader::Stage stage) noexcept
        -> tl::expected<std::vector<uint32_t>, Error>;

    struct ShaderCreateInfo
    {
        std::span<uint32_t> irCode;
        Shader::Stage stage;
        std::filesystem::path compileDirectory;
    };
}  // namespace exage::Graphics
