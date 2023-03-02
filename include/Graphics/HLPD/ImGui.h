#pragma once

#include "Graphics/Context.h"
#include "Graphics/Texture.h"

namespace exage::Graphics::HLPD
{
	struct ImGuiInitInfo
    {
        Context& context;
        Queue& queue;
        Window& window;
        Texture::Format outputFormat;
    };

    void initImGui(ImGuiInitInfo& initInfo) noexcept;
}