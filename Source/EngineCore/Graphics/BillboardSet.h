//
// Copyright (c) 2008-2017 the Urho3D project.
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

#include "../Graphics/Drawable.h"
#include "../IO/VectorBuffer.h"
#include "../Math/Color.h"
#include "../Math/Matrix3x4.h"
#include "../Math/Rect.h"

namespace Atomic
{

class IndexBuffer;
class VertexBuffer;

// ATOMIC BEGIN
/// One billboard in the billboard set.
class ATOMIC_API Billboard : public RefCounted
{
    friend class BillboardSet;
    friend class ParticleEmitter;

    ATOMIC_REFCOUNTED(Billboard);

public:
    Billboard();
    virtual ~Billboard();

    const Vector3& GetPosition() const { return position_; }
    void SetPosition(const Vector3 &position) { position_ = position; }

    const Vector2 GetSize() const { return size_; }
    void SetSize(const Vector2 &size) { size_ = size; }

    const Rect& GetUV() const { return uv_; }
    void SetUV(const Rect &uv) { uv_ = uv; }

    const Color& GetColor() const { return color_; }
    void SetColor(const Color &color) { color_ = color; }

    float GetRotation() const { return rotation_; }
    void SetRotation(float rotation) { rotation_ = rotation; }

    const Vector3& GetDirection() const { return direction_; }
    void SetDirection(const Vector3& direction) { direction_ = direction; }

    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

    float GetSortDistance() const { return sortDistance_; }
    void SetSortDistance(float sortDistance) { sortDistance_ = sortDistance; }
// ATOMIC END

    /// Position.
    Vector3 position_;
    /// Two-dimensional size. If BillboardSet has fixed screen size enabled, this is measured in pixels instead of world units.
    Vector2 size_;
    /// UV coordinates.
    Rect uv_;
    /// Color.
    Color color_;
    /// Rotation.
    float rotation_;
    /// Direction (For direction based billboard only).
    Vector3 direction_;
    /// Enabled flag.
    bool enabled_;
    /// Sort distance. Used internally.
    float sortDistance_;
    /// Scale factor for fixed screen size mode. Used internally.
    float screenScaleFactor_;
};

// ATOMIC BEGIN
static const unsigned MAX_BILLBOARDS = 65536 / 4;
// ATOMIC END

/// %Billboard component.
class ATOMIC_API BillboardSet : public Drawable
{
    ATOMIC_OBJECT(BillboardSet, Drawable);

public:
    /// Construct.
    BillboardSet(Context* context);
    /// Destruct.
    virtual ~BillboardSet();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Process octree raycast. May be called from a worker thread.
    virtual void ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results);
    /// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
    virtual void UpdateBatches(const FrameInfo& frame);
    /// Prepare geometry for rendering. Called from a worker thread if possible (no GPU update.)
    virtual void UpdateGeometry(const FrameInfo& frame);
    /// Return whether a geometry update is necessary, and if it can happen in a worker thread.
    virtual UpdateGeometryType GetUpdateGeometryType();

    /// Set material.
    void SetMaterial(Material* material);
    /// Set number of billboards.
    void SetNumBillboards(unsigned num);
    /// Set whether billboards are relative to the scene node. Default true.
    void SetRelative(bool enable);
    /// Set whether scene node scale affects billboards' size. Default true.
    void SetScaled(bool enable);
    /// Set whether billboards are sorted by distance. Default false.
    void SetSorted(bool enable);
    /// Set whether billboards have fixed size on screen (measured in pixels) regardless of distance to camera. Default false.
    void SetFixedScreenSize(bool enable);
    /// Set how the billboards should rotate in relation to the camera. Default is to follow camera rotation on all axes (FC_ROTATE_XYZ.)
    void SetFaceCameraMode(FaceCameraMode mode);
    /// Set minimal angle between billboard normal and look-at direction.
    void SetMinAngle(float angle);
    /// Set animation LOD bias.
    void SetAnimationLodBias(float bias);
    /// Mark for bounding box and vertex buffer update. Call after modifying the billboards.
    void Commit();

    /// Return material.
    Material* GetMaterial() const;

    /// Return number of billboards.
    unsigned GetNumBillboards() const { return billboards_.Size(); }

    // ATOMIC BEGIN
    /// Return all billboards.
    Vector<SharedPtr<Billboard>>& GetBillboards() { return billboards_; }
    // ATOMIC END

    /// Return billboard by index.
    Billboard* GetBillboard(unsigned index);

    /// Return whether billboards are relative to the scene node.
    bool IsRelative() const { return relative_; }

    /// Return whether scene node scale affects billboards' size.
    bool IsScaled() const { return scaled_; }

    /// Return whether billboards are sorted.
    bool IsSorted() const { return sorted_; }

    /// Return whether billboards are fixed screen size.
    bool IsFixedScreenSize() const { return fixedScreenSize_; }

    /// Return how the billboards rotate in relation to the camera.
    FaceCameraMode GetFaceCameraMode() const { return faceCameraMode_; }

    /// Return minimal angle between billboard normal and look-at direction.
    float GetMinAngle() const { return minAngle_; }

    /// Return animation LOD bias.
    float GetAnimationLodBias() const { return animationLodBias_; }

    /// Set material attribute.
    void SetMaterialAttr(const ResourceRef& value);
    /// Set billboards attribute.
    void SetBillboardsAttr(const VariantVector& value);
    /// Set billboards attribute for network replication.
    void SetNetBillboardsAttr(const PODVector<unsigned char>& value);
    /// Return material attribute.
    ResourceRef GetMaterialAttr() const;
    /// Return billboards attribute.
    VariantVector GetBillboardsAttr() const;
    /// Return billboards attribute for network replication.
    const PODVector<unsigned char>& GetNetBillboardsAttr() const;

protected:
    /// Recalculate the world-space bounding box.
    virtual void OnWorldBoundingBoxUpdate();
    /// Mark billboard vertex buffer to need an update.
    void MarkPositionsDirty();

    /// Billboards.
    // ATOMIC BEGIN
    Vector<SharedPtr<Billboard>> billboards_;
    // ATOMIC END
    /// Animation LOD bias.
    float animationLodBias_;
    /// Animation LOD timer.
    float animationLodTimer_;
    /// Billboards relative flag.
    bool relative_;
    /// Scale affects billboard scale flag.
    bool scaled_;
    /// Billboards sorted flag.
    bool sorted_;
    /// Billboards fixed screen size flag.
    bool fixedScreenSize_;
    /// Billboard rotation mode in relation to the camera.
    FaceCameraMode faceCameraMode_;
    /// Minimal angle between billboard normal and look-at direction.
    float minAngle_;

private:
    /// Resize billboard vertex and index buffers.
    void UpdateBufferSize();
    /// Rewrite billboard vertex buffer.
    void UpdateVertexBuffer(const FrameInfo& frame);
    /// Calculate billboard scale factors in fixed screen size mode.
    void CalculateFixedScreenSize(const FrameInfo& frame);

    /// Geometry.
    SharedPtr<Geometry> geometry_;
    /// Vertex buffer.
    SharedPtr<VertexBuffer> vertexBuffer_;
    /// Index buffer.
    SharedPtr<IndexBuffer> indexBuffer_;
    /// Transform matrices for position and billboard orientation.
    Matrix3x4 transforms_[2];
    /// Buffers need resize flag.
    bool bufferSizeDirty_;
    /// Vertex buffer needs rewrite flag.
    bool bufferDirty_;
    /// Force update flag (ignore animation LOD momentarily.)
    bool forceUpdate_;
    /// Update billboard geometry type
    bool geometryTypeUpdate_;
    /// Sorting flag. Triggers a vertex buffer rewrite for each view this billboard set is rendered from.
    bool sortThisFrame_;
    /// Whether was last rendered from an ortho camera.
    bool hasOrthoCamera_;
    /// Frame number on which was last sorted.
    unsigned sortFrameNumber_;
    /// Previous offset to camera for determining whether sorting is necessary.
    Vector3 previousOffset_;
    /// Billboard pointers for sorting.
    Vector<Billboard*> sortedBillboards_;
    /// Attribute buffer for network replication.
    mutable VectorBuffer attrBuffer_;
};

}
