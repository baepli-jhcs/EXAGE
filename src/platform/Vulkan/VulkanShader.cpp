#include <fstream>

#include "exage/platform/Vulkan/VulkanShader.h"

#include "alpaca/alpaca.h"

namespace exage::Graphics
{
    struct CacheMetadata
    {
        std::string deviceName;  // If the device name changes, the cache is invalidated
        uint64_t lastWriteTime{};  // In seconds, if the file is newer, the cache is invalidated
    };

    auto VulkanShader::create(VulkanContext& context, const ShaderCreateInfo& createInfo) noexcept
        -> tl::expected<VulkanShader, Error>
    {
        VulkanShader shader(context, createInfo.stage);
        std::optional<Error> error = shader.init(createInfo);
        if (error.has_value())
        {
            return tl::make_unexpected(*error);
        }
        return shader;
    }

    VulkanShader::~VulkanShader()
    {
        if (_shaderModule)
        {
            _context.get().getDevice().destroyShaderModule(_shaderModule);
        }
    }

    VulkanShader::VulkanShader(VulkanShader&& old) noexcept
        : Shader(std::move(old))
        , _context(old._context)
        , _shaderModule(std::exchange(old._shaderModule, nullptr))
    {
    }

    auto VulkanShader::operator=(VulkanShader&& old) noexcept -> VulkanShader&
    {
        if (this != &old)
        {
            Shader::operator=(std::move(old));
            _context = old._context;
            _shaderModule = std::exchange(old._shaderModule, nullptr);
        }
        return *this;
    }

    auto VulkanShader::init(const ShaderCreateInfo& createInfo) noexcept -> std::optional<Error>
    {
        tl::expected bufferResult = cacheCompile(createInfo);
        if (!bufferResult.has_value())
        {
            return bufferResult.error();
        }

        vk::ShaderModuleCreateInfo shaderCreateInfo {};
        shaderCreateInfo.codeSize = bufferResult->size();
        shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(bufferResult->data());
        checkVulkan(_context.get().getDevice().createShaderModule(
            &shaderCreateInfo, nullptr, &_shaderModule));
        return std::nullopt;
    }

    auto VulkanShader::cacheCompile(const ShaderCreateInfo& createInfo) noexcept
        -> tl::expected<std::vector<uint8_t>, Error>
    {
        if (!std::filesystem::exists(createInfo.path))
        {
            return tl::make_unexpected(FileError::eFileNotFound);
        }

        std::filesystem::file_time_type const lastWriteTimeClock =
            std::filesystem::last_write_time(createInfo.path);
        uint64_t lastWriteTime =
           static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(lastWriteTimeClock.time_since_epoch())
                .count());
        std::string deviceName = _context.get().getPhysicalDevice().getProperties().deviceName;

        std::filesystem::path metadataPath = createInfo.cachePath;
        metadataPath.replace_extension("metadata");

        if (!createInfo.forceRecompile)
        {
            std::vector<uint8_t> cacheBuffer;
            auto tryCache = [&]() -> bool
            {
                if (!std::filesystem::exists(metadataPath))
                {
                    return false;
                }

                std::ifstream metadataFile(metadataPath, std::ios::binary);
                if (metadataFile.is_open())
                {
                    return false;
                }

                std::vector<uint8_t> metadataBuffer;
                metadataBuffer.resize(std::filesystem::file_size(metadataPath));
                metadataFile.read(reinterpret_cast<char*>(metadataBuffer.data()),
                                  metadataBuffer.size());
                metadataFile.close();

                std::error_code ec;
                auto const metadata = alpaca::deserialize<CacheMetadata>(metadataBuffer, ec);

                if (ec)
                {
                    return false;
                }

                if (metadata.lastWriteTime != lastWriteTime || metadata.deviceName != deviceName)
                {
                    return false;
                }
                const std::filesystem::path& cachePath = createInfo.cachePath;
                if (!std::filesystem::exists(cachePath))
                {
                    return false;
                }
                std::ifstream cacheFile(cachePath, std::ios::binary);
                if (!cacheFile.is_open())
                {
                    return false;
                }
                cacheBuffer.resize(std::filesystem::file_size(cachePath));
                cacheFile.read(reinterpret_cast<char*>(cacheBuffer.data()), cacheBuffer.size());
                cacheFile.close();
                return true;
            };

            if (tryCache())
            {
                return cacheBuffer;
            }
        }

        // If we get here, we need to compile the shader using glslang
        std::vector<uint8_t> buffer;

        std::string const source;

        // Write the cache
        if (createInfo.cache)
        {
            std::filesystem::path metadataPath = createInfo.cachePath;
            metadataPath.replace_extension("metadata");
            std::ofstream metadataFile(metadataPath, std::ios::binary);

            CacheMetadata metadata;
            metadata.deviceName = deviceName;
            metadata.lastWriteTime = lastWriteTime;

            std::vector<uint8_t> metadataBuffer;
            alpaca::serialize(metadata, metadataBuffer);
            metadataFile.write(reinterpret_cast<const char*>(metadataBuffer.data()),
                               metadataBuffer.size());
            metadataFile.close();

            const std::filesystem::path& cachePath = createInfo.cachePath;
            std::ofstream cacheFile(cachePath, std::ios::binary);
            cacheFile.write(reinterpret_cast<const char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
            cacheFile.close();
        }

        return buffer;
    }
}  // namespace exage::Graphics
