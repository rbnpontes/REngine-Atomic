#pragma once
#include "./DriverInstance.h"
#include "./PipelineStateBuilder.h"
#include "./ShaderProgram.h"

#include "../Graphics/GraphicsDefs.h"
#include "../Math/Rect.h"
#include "../Graphics/Graphics.h"

#include <DiligentCore/Common/interface/RefCntAutoPtr.hpp>
#include <DiligentCore/Graphics/GraphicsEngine/interface/Buffer.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/PipelineState.h>
#include <DiligentCore/Graphics/GraphicsEngine/interface/ShaderResourceBinding.h>

namespace REngine
{
    enum class RenderCommandDirtyState : unsigned
    {
        none            = 0x0,
        render_targets  = 1 << 0,
        pipeline        = 1 << 1,
        textures        = 1 << 2,
        shader_program  = 1 << 3,
        viewport        = 1 << 4,
        scissor         = 1 << 5,
        vertex_buffer   = 1 << 6,
        index_buffer    = 1 << 7,
        vertex_decl     = 1 << 8,
        srb             = 1 << 9,
        commit_pipeline = 1 << 10,
        commit_srb      = 1 << 11,
        all = render_targets | pipeline
        | textures | shader_program | viewport
        | scissor | vertex_buffer | index_buffer | vertex_decl
        | srb | commit_srb
    };

    enum class RenderCommandSkipFlags : unsigned
    {
        none = 0x0,
        pipeline_build = 1 << 0,
        srb_build = 1 << 1
    };

    struct ShaderParameterUpdateData
    {
		Atomic::StringHash name{Atomic::StringHash::ZERO};
        Atomic::Variant value{Atomic::Variant::EMPTY};
	};

    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Atomic::Variant* value);
}
