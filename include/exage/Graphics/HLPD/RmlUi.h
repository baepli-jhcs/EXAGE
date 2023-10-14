#pragma once

#include <memory_resource>
#include <variant>

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Vertex.h>
#include <glm/glm.hpp>
#include <robin_hood.h>

#include "exage/System/Cursor.h"
#include "exage/System/Window.h"
#include "exage/utils/classes.h"

namespace exage::Graphics::RmlUi
{
    struct RenderInstruction
    {
        int numVertices;
        int numIndices;
        size_t baseVertex;
        size_t baseIndex;
        Rml::TextureHandle texture;
        Rml::Vector2f translation;
    };

    struct CompileInstruction
    {
        int numVertices;
        int numIndices;
        size_t baseVertex;
        size_t baseIndex;
        Rml::TextureHandle texture;

        Rml::CompiledGeometryHandle compiledGeometryHandle;
    };

    struct RenderCompiledInstruction
    {
        Rml::CompiledGeometryHandle geometry;
        Rml::Vector2f translation;
    };

    struct ReleaseCompiledInstruction
    {
        Rml::CompiledGeometryHandle geometry;
    };

    struct EnableScissorInstruction
    {
        bool enable;
    };

    struct SetScissorInstruction
    {
        int x;
        int y;
        int width;
        int height;
    };

    struct GenerateTextureInstruction
    {
        Rml::TextureHandle textureHandle;
        Rml::Vector2i sourceDimensions;
        size_t baseIndex;
    };

    struct ReleaseTextureInstruction
    {
        Rml::TextureHandle textureHandle;
    };

    struct SetTransformInstruction
    {
        glm::mat4 transform;
    };

    using Instruction = std::variant<RenderInstruction,
                                     CompileInstruction,
                                     RenderCompiledInstruction,
                                     ReleaseCompiledInstruction,
                                     EnableScissorInstruction,
                                     SetScissorInstruction,
                                     GenerateTextureInstruction,
                                     ReleaseTextureInstruction,
                                     SetTransformInstruction>;

    /* This interface doesn't actually render anything, it just stores the instructions in a vector
        to pass to the rendering thread */
    class RenderInterface : public Rml::RenderInterface
    {
      public:
        RenderInterface() noexcept = default;
        ~RenderInterface() override = default;

        EXAGE_DELETE_COPY(RenderInterface);
        EXAGE_DELETE_MOVE(RenderInterface);

        void RenderGeometry(Rml::Vertex* vertices,
                            int num_vertices,
                            int* indices,
                            int num_indices,
                            Rml::TextureHandle texture,
                            const Rml::Vector2f& translation) override;

        auto CompileGeometry(Rml::Vertex* vertices,
                             int num_vertices,
                             int* indices,
                             int num_indices,
                             Rml::TextureHandle texture) -> Rml::CompiledGeometryHandle override;
        void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry,
                                    const Rml::Vector2f& translation) override;
        void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) override;

        void EnableScissorRegion(bool enable) override;
        void SetScissorRegion(int x, int y, int width, int height) override;

        auto LoadTexture(Rml::TextureHandle& texture_handle,
                         Rml::Vector2i& texture_dimensions,
                         const Rml::String& source) -> bool override;
        auto GenerateTexture(Rml::TextureHandle& texture_handle,
                             const Rml::byte* source,
                             const Rml::Vector2i& source_dimensions) -> bool override;
        void ReleaseTexture(Rml::TextureHandle texture_handle) override;

        void SetTransform(const Rml::Matrix4f* transform) override;

        /* EXAGE Functions */
        void beginNewFrame() noexcept;

        auto getVertices() noexcept -> std::pmr::vector<Rml::Vertex>& { return _vertices; }
        auto getIndices() noexcept -> std::pmr::vector<int>& { return _indices; }
        auto getTextureData() noexcept -> std::pmr::vector<Rml::byte>& { return _textureData; }
        auto getInstructions() noexcept -> std::pmr::vector<Instruction>& { return _instructions; }

      private:
        std::pmr::vector<Rml::Vertex> _vertices;
        std::pmr::vector<int> _indices;
        std::pmr::vector<Rml::byte> _textureData;
        std::pmr::vector<Instruction> _instructions;

        uintptr_t _currentTexture = 0;
        uintptr_t _currentCompiledGeometry = 0;

        auto getNextTexture() noexcept -> Rml::TextureHandle
        {
            return reinterpret_cast<Rml::TextureHandle>(_currentTexture++);
        }

        auto getNextCompiledGeometry() noexcept -> Rml::CompiledGeometryHandle
        {
            return reinterpret_cast<Rml::CompiledGeometryHandle>(_currentCompiledGeometry++);
        }
    };

    class SystemInterface : public Rml::SystemInterface
    {
      public:
        explicit SystemInterface(System::WindowAPI api) noexcept;
        ~SystemInterface() override = default;

        EXAGE_DELETE_COPY(SystemInterface);
        EXAGE_DELETE_MOVE(SystemInterface);

        auto GetElapsedTime() -> double override;

        void SetMouseCursor(const Rml::String& cursor_name) override;

        void SetClipboardText(const Rml::String& text) override;
        void GetClipboardText(Rml::String& text) override;

      private:
        System::WindowAPI _api;

        std::unique_ptr<System::Cursor> _arrow;
        std::unique_ptr<System::Cursor> _pointer;
        std::unique_ptr<System::Cursor> _crosshair;
        std::unique_ptr<System::Cursor> _text;
    };
}  // namespace exage::Graphics::RmlUi