//
// Copyright (c) 2014-2016, THUNDERBEAST GAMES LLC All rights reserved
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include <Atomic/Core/Variant.h>
#include <Atomic/Resource/ResourceCache.h>

#include "ScriptSystem.h"

namespace Atomic
{
    /// Wraps a Variant as a RefCounted so we can easily send it to script code
    /// For performance sensitive code, specialized marshaling should be used instead
    class ScriptVariant : public RefCounted
    {
        ATOMIC_REFCOUNTED(ScriptVariant)

    public:

        ScriptVariant() : RefCounted()
        {

        }

        ScriptVariant(const Variant& source) : RefCounted()
        {
            variant_ = source;
        }

        virtual ~ScriptVariant()
        {

        }

        const Variant& GetVariant() const { return variant_; }

        void SetVariant(const Variant& value) { variant_ = value; }

        bool GetBool() const { return variant_.GetBool(); }

        void SetBool(bool value) { variant_ = value; }

        int GetInt() const { return variant_.GetInt(); }

        void SetInt(int value) { variant_ = value; }

        unsigned GetUInt() const { return variant_.GetUInt(); }

        void SetUInt(unsigned value) { variant_ = value; }

        float GetFloat() const { return variant_.GetFloat(); }

        void SetFloat(float value) { variant_ = value; }

        const Vector2& GetVector2() const { return variant_.GetVector2(); }

        void SetVector2(const Vector2& value) { variant_ = value; }

        const Vector3& GetVector3() const { return variant_.GetVector3(); }

        Resource* GetResource() const;

        void SetResource(Resource* resource);

        void SetVector3(const Vector3& value) { variant_ = value; }

        const Quaternion& GetQuaternion() const { return variant_.GetQuaternion(); }

        void SetQuaternion(const Quaternion& value) { variant_ = value; }

        const Vector4& GetVector4() const { return variant_.GetVector4(); }

        void SetVector4(const Vector4& value) { variant_ = value; }

        const Color& GetColor() const { return variant_.GetColor(); }

        void SetColor(const Color& value) { variant_ = value; }

        const String& GetString() const { return variant_.GetString(); }

        void SetString(const String& value) { variant_ = value; }

        const VariantVector& GetVariantVector() const { return variant_.GetVariantVector(); }

        void SetVariantVector(const VariantVector& value) { variant_ = value; }

        const VariantMap& GetVariantMap() const { return variant_.GetVariantMap(); }

        void SetVariantMap(const VariantMap& value) { variant_ = value; }

    private:

        Variant variant_;

    };

}
