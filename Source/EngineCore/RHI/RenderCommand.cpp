#include "./RenderCommand.h"

#include "./DiligentUtils.h"
#include "../IO/Log.h"
#include "../Core/Profiler.h"

namespace REngine
{
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const float* data, uint32_t count)
    {
        buffer->SetParameter(offset, count * sizeof(float), data);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, float value)
    {
        buffer->SetParameter(offset, sizeof(float), &value);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, int value)
    {
        buffer->SetParameter(offset, sizeof(int), &value);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, bool value)
    {
        buffer->SetParameter(offset, sizeof(bool), &value);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Color& value)
    {
	    buffer->SetParameter(offset, sizeof(float) * 4, value.Data());
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Vector2& value)
    {
        buffer->SetParameter(offset, sizeof(Vector2), value.Data());
	}
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Vector3& value)
    {
        buffer->SetParameter(offset, sizeof(Vector3), value.Data());
	}
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Vector4& value)
    {
        buffer->SetParameter(offset, sizeof(Vector4), value.Data());
	}
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Matrix3x4& value)
    {
        buffer->SetParameter(offset, sizeof(Matrix3x4), &value);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Matrix3& value)
    {
        const Matrix3x4 m(value);
        render_command_write_param(buffer, offset, m);
    }
    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Matrix4& value)
    {
        buffer->SetParameter(offset, sizeof(Matrix4), &value);
	}
#define WRITE_PARAM(type, accessor) \
    {\
        const type& x = value->accessor(); \
        buffer->SetParameter(offset, sizeof(type), &x); \
    }

    void render_command_write_param(Atomic::ConstantBuffer* buffer, uint32_t offset, const Variant* value)
    {
	    switch(value->GetType())
	    {
	    case VAR_FLOATVECTOR:
		    {
                const auto& float_vector = value->GetFloatVector();
                buffer->SetParameter(offset, float_vector.Size() * sizeof(float), float_vector.Buffer());
		    }
            break;
		case VAR_FLOAT:
            WRITE_PARAM(float, GetFloat);
		    break;
        case VAR_INT:
            WRITE_PARAM(int, GetInt);
            break;
        case VAR_BOOL:
            WRITE_PARAM(bool, GetBool);
            break;
	    case VAR_COLOR:
            WRITE_PARAM(Color, GetColor);
            break;
	    case VAR_VECTOR2:
            WRITE_PARAM(Vector2, GetVector2);
            break;
	    case VAR_VECTOR3:
            WRITE_PARAM(Vector3, GetVector3);
            break;
	    case VAR_VECTOR4:
            WRITE_PARAM(Vector4, GetVector4);
            break;
	    case VAR_MATRIX3X4:
            WRITE_PARAM(Matrix3x4, GetMatrix3x4);
            break;
	    case VAR_MATRIX3:
		    {
	    		const Matrix3x4 m(value->GetMatrix3());
                buffer->SetParameter(offset, sizeof(Matrix3x4), &m);
		    }
            break;
	    case VAR_MATRIX4:
            WRITE_PARAM(Matrix4, GetMatrix4);
			break;
default: ;
	    }
    }
}
