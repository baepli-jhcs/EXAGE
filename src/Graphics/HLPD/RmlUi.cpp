#include <cstddef>

#include "exage/Graphics/HLPD/RmLui.h"

#include <glm/gtc/type_ptr.hpp>

#include "exage/Graphics/HLPD/RmlUi.h"

namespace exage::Graphics::RmlUi
{

    void RenderInterface::RenderGeometry(Rml::Vertex* vertices,
                                         int num_vertices,
                                         int* indices,
                                         int num_indices,
                                         Rml::TextureHandle texture,
                                         const Rml::Vector2f& translation)
    {
        RenderInstruction instruction;
        instruction.numVertices = num_vertices;
        instruction.numIndices = num_indices;
        instruction.baseVertex = _vertices.size();
        instruction.baseIndex = _indices.size();
        instruction.texture = texture;
        instruction.translation = translation;

        _vertices.insert(_vertices.end(), vertices, vertices + num_vertices);
        _indices.insert(_indices.end(), indices, indices + num_indices);
        _instructions.push_back(instruction);
    }

    auto RenderInterface::CompileGeometry(Rml::Vertex* vertices,
                                          int num_vertices,
                                          int* indices,
                                          int num_indices,
                                          Rml::TextureHandle texture) -> Rml::CompiledGeometryHandle
    {
        CompileInstruction instruction {};
        instruction.numVertices = num_vertices;
        instruction.numIndices = num_indices;
        instruction.baseVertex = _vertices.size();
        instruction.baseIndex = _indices.size();
        instruction.texture = texture;
        instruction.compiledGeometryHandle = getNextCompiledGeometry();

        _vertices.insert(_vertices.end(), vertices, vertices + num_vertices);
        _indices.insert(_indices.end(), indices, indices + num_indices);
        _instructions.push_back(instruction);

        return instruction.compiledGeometryHandle;
    }

    void RenderInterface::RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry,
                                                 const Rml::Vector2f& translation)
    {
        RenderCompiledInstruction instruction;
        instruction.geometry = geometry;
        instruction.translation = translation;

        _instructions.push_back(instruction);
    }

    void RenderInterface::ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry)
    {
        ReleaseCompiledInstruction instruction;
        instruction.geometry = geometry;

        _instructions.push_back(instruction);
    }

    void RenderInterface::EnableScissorRegion(bool enable)
    {
        EnableScissorInstruction instruction;
        instruction.enable = enable;

        _instructions.push_back(instruction);
    }

    void RenderInterface::SetScissorRegion(int x, int y, int width, int height)
    {
        SetScissorInstruction instruction;
        instruction.x = x;
        instruction.y = y;
        instruction.width = width;
        instruction.height = height;

        _instructions.push_back(instruction);
    }

    bool RenderInterface::LoadTexture(Rml::TextureHandle& texture_handle,
                                      Rml::Vector2i& texture_dimensions,
                                      const Rml::String& source)
    {
        // TODO: implement
        return false;
    }

    bool RenderInterface::GenerateTexture(Rml::TextureHandle& texture_handle,
                                          const Rml::byte* source,
                                          const Rml::Vector2i& source_dimensions)
    {
        texture_handle = getNextTexture();

        GenerateTextureInstruction instruction;
        instruction.textureHandle = texture_handle;
        instruction.sourceDimensions = source_dimensions;
        instruction.baseIndex = _textureData.size();

        size_t const numPixels =
            static_cast<const size_t>(source_dimensions.x) * source_dimensions.y;

        _textureData.insert(_textureData.end(), source, source + numPixels * 4);
        _instructions.push_back(instruction);

        return true;
    }

    void RenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
    {
        ReleaseTextureInstruction instruction {};
        instruction.textureHandle = texture_handle;

        _instructions.push_back(instruction);
    }

    void RenderInterface::SetTransform(const Rml::Matrix4f* transform)
    {
        SetTransformInstruction instruction {};
        if (transform != nullptr)
        {
            instruction.transform = glm::make_mat4x4(transform->data());
        }
        else
        {
            instruction.transform = glm::mat4(1.0f);
        }

        _instructions.push_back(instruction);
    }

    void RenderInterface::beginNewFrame() noexcept
    {
        _instructions.clear();
        _vertices.clear();
        _indices.clear();
        _textureData.clear();
    }
}  // namespace exage::Graphics::RmlUi