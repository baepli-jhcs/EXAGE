#include <fstream>

#include "exage/Projects/Project.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_set.hpp>

#include "exage/Core/Errors.h"

namespace exage::Projects
{
    auto loadProject(const std::filesystem::path& path) noexcept -> tl::expected<Project, Error>
    {
        try
        {
            std::ifstream file(path, std::ios::binary);
            cereal::BinaryInputArchive archive(file);

            Project project;
            archive(project);

            return project;
        }
        catch (const std::exception& e)
        {
            return tl::make_unexpected(Errors::DeserializationFailed());
        }
    }

    auto saveProject(const std::filesystem::path& path, const Project& project) noexcept
        -> tl::expected<void, Error>
    {
        try
        {
            std::ofstream file(path, std::ios::binary);
            cereal::BinaryOutputArchive archive(file);

            archive(project);

            return {};
        }
        catch (const std::exception& e)
        {
            return tl::make_unexpected(Errors::SerializationFailed());
        }
    }
}  // namespace exage::Projects