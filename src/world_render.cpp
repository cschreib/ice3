#include "world.hpp"
#include "camera.hpp"
#include "application.hpp"
#include "shader.hpp"
#include <GL/glew.h>
#include <GL/gl.h>

void world::render()
{
    sf::Clock c;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glFrontFace(GL_CW);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_CULL_FACE);

    if (bWireframe_)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (bFog_)
    {
        glEnable(GL_FOG);
        float pFogcolor[] = {mSkyColor_.r, mSkyColor_.g, mSkyColor_.b, mSkyColor_.a};
        glFogfv(GL_FOG_COLOR, pFogcolor);
        glHint(GL_FOG_HINT, GL_FASTEST);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, fRenderDistance_*0.6f);
        glFogf(GL_FOG_END, fRenderDistance_);
    }

    pCamera_->set_far_distance(10.0f*fRenderDistance_);
    pCamera_->set_aspect_ratio((float)mApp_.get_window().getSize().x/(float)mApp_.get_window().getSize().y);
    pCamera_->bind();

    glViewport(0, 0, mApp_.get_window().getSize().x, mApp_.get_window().getSize().y);

    glClearColor(mSkyColor_.r, mSkyColor_.g, mSkyColor_.b, mSkyColor_.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    render_chunks_();
    render_selected_block_();

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    if (bWireframe_)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (bFog_)
        glDisable(GL_FOG);

    pGUIManager_->render_ui();

    mRenderData_.fRenderTime += c.getElapsedTime().asSeconds();
}

void world::render_chunks_() const
{
    if (!pCurrentChunk_)
        return;

    mRenderData_.uiNumRenderedVertex = 0u;

    glEnable(GL_TEXTURE_2D);

    if (bUseShaders_)
    {
        pBlockShader_->bind();
        pBlockShader_->bind_texture("texture", block::BLOCK_TEXTURE);
        pBlockShader_->bind_texture("light",   lLightingArray_);
    }
    else
        block::BLOCK_TEXTURE->bind();

    if (!bUseVBO_)
        glBegin(GL_QUADS);

    for (auto chunk : lVisibleChunkList_)
    {
        if (!chunk->is_normal_vertex_cache_empty())
            render_chunk_(*chunk);
    }

    if (!bUseVBO_)
        glEnd();

    // Render two sided blocks
    glDisable(GL_CULL_FACE);

    if (!bUseVBO_)
        glBegin(GL_QUADS);

    for (auto chunk : lVisibleChunkList_)
    {
        if (!chunk->is_two_sided_vertex_cache_empty())
            render_chunk_two_sided_(*chunk);
    }

    if (!bUseVBO_)
        glEnd();

    // Render alpha blended blocks
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!bUseVBO_)
        glBegin(GL_QUADS);

    for (auto chunk : lVisibleChunkList_)
    {
        if (!chunk->is_alpha_blended_vertex_cache_empty())
            render_chunk_alpha_blended_(*chunk);
    }

    if (!bUseVBO_)
        glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

    if (bUseShaders_)
        pBlockShader_->unbind();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

void world::render_chunk_(const block_chunk& mChunk) const
{
    if (bUseVBO_)
    {
        if (!mChunk.pNormalVBO_)
            return;

        vector3f mPosition = mChunk.get_position(pCurrentChunk_);

        glPushMatrix();
        glTranslatef(mPosition.x, mPosition.y, mPosition.z);

        mRenderData_.uiNumRenderedVertex += mChunk.pNormalVBO_->get_size();
        mChunk.pNormalVBO_->render();

        glPopMatrix();
    }
    else
    {
        if (!bSmoothLighting_)
        {
            if (bUseShaders_)
                render_faces_shader_(mChunk.mVertexCache_.lData, 0, mChunk.mVertexCache_.uiNumNormalVertex);
            else
                render_faces_(mChunk.mVertexCache_.lData, 0, mChunk.mVertexCache_.uiNumNormalVertex);
        }
        else
        {
            if (bUseShaders_)
                render_faces_shader_smooth_(mChunk.mVertexCache_.lData, 0, mChunk.mVertexCache_.uiNumNormalVertex);
            else
                render_faces_smooth_(mChunk.mVertexCache_.lData, 0, mChunk.mVertexCache_.uiNumNormalVertex);
        }
    }
}

void world::render_chunk_two_sided_(const block_chunk& mChunk) const
{
    if (bUseVBO_)
    {
        if (!mChunk.pTwoSidedVBO_)
            return;

        vector3f mPosition = mChunk.get_position(pCurrentChunk_);

        glPushMatrix();
        glTranslatef(mPosition.x, mPosition.y, mPosition.z);

        mRenderData_.uiNumRenderedVertex += mChunk.pTwoSidedVBO_->get_size();
        mChunk.pTwoSidedVBO_->render();

        glPopMatrix();
    }
    else
    {
        size_t offset = mChunk.mVertexCache_.uiNumNormalVertex;
        if (!bSmoothLighting_)
        {
            if (bUseShaders_)
                render_faces_shader_(mChunk.mVertexCache_.lData, offset, mChunk.mVertexCache_.uiNumTwoSidedVertex);
            else
                render_faces_(mChunk.mVertexCache_.lData, offset, mChunk.mVertexCache_.uiNumTwoSidedVertex);
        }
        else
        {
            if (bUseShaders_)
                render_faces_shader_smooth_(mChunk.mVertexCache_.lData, offset, mChunk.mVertexCache_.uiNumTwoSidedVertex);
            else
                render_faces_smooth_(mChunk.mVertexCache_.lData, offset, mChunk.mVertexCache_.uiNumTwoSidedVertex);
        }
    }
}

void world::render_chunk_alpha_blended_(const block_chunk& mChunk) const
{
    if (bUseVBO_)
    {
        if (!mChunk.pAlphaBlendedVBO_)
            return;

        vector3f mPosition = mChunk.get_position(pCurrentChunk_);

        glPushMatrix();
        glTranslatef(mPosition.x, mPosition.y, mPosition.z);

        mRenderData_.uiNumRenderedVertex += mChunk.pAlphaBlendedVBO_->get_size();
        mChunk.pAlphaBlendedVBO_->render();

        glPopMatrix();
    }
    else
    {
        size_t offset = mChunk.mVertexCache_.uiNumNormalVertex + mChunk.mVertexCache_.uiNumTwoSidedVertex;
        if (!bSmoothLighting_)
        {
            if (bUseShaders_)
                render_faces_shader_(mChunk.mVertexCache_.lData, offset, mChunk.mVertexCache_.uiNumAlphaBlendedVertex);
            else
                render_faces_(mChunk.mVertexCache_.lData, offset, mChunk.mVertexCache_.uiNumAlphaBlendedVertex);
        }
        else
        {
            if (bUseShaders_)
                render_faces_shader_smooth_(mChunk.mVertexCache_.lData, offset, mChunk.mVertexCache_.uiNumAlphaBlendedVertex);
            else
                render_faces_smooth_(mChunk.mVertexCache_.lData, offset, mChunk.mVertexCache_.uiNumAlphaBlendedVertex);
        }
    }
}

void world::render_faces_(const std::vector<block_face>& data, size_t offset, size_t num) const
{
    mRenderData_.uiNumRenderedVertex += 4*data.size();

    for (auto& mFace : data)
    {
        color mColor = block::HUE_TABLE[mFace.hue];
        if (mFace.b == pSelectedBlock_)
            mColor = mColor*color::RED;

        const texture::color& mLight = lLightingArray_->get_pixel(
            mFace.sunlight/uiLightingArrayRatio_, mFace.light/uiLightingArrayRatio_
        );

        mColor.r *= mLight.r/255.0f;
        mColor.g *= mLight.g/255.0f;
        mColor.b *= mLight.b/255.0f;

        glColor4fv(reinterpret_cast<const float*>(&mColor));

        render_vertex_(mFace.v1);
        render_vertex_(mFace.v2);
        render_vertex_(mFace.v3);
        render_vertex_(mFace.v4);
    }
}

void world::render_faces_shader_(const std::vector<block_face>& data, size_t offset, size_t num) const
{
    mRenderData_.uiNumRenderedVertex += 4*data.size();

    for (auto& mFace : data)
    {
        color mColor = block::HUE_TABLE[mFace.hue];
        if (mFace.b == pSelectedBlock_)
            mColor = mColor*color::RED;

        glColor4fv(reinterpret_cast<float*>(&mColor));

        render_vertex_shader_(mFace.v1, mFace);
        render_vertex_shader_(mFace.v2, mFace);
        render_vertex_shader_(mFace.v3, mFace);
        render_vertex_shader_(mFace.v4, mFace);
    }
}

void world::render_faces_smooth_(const std::vector<block_face>& data, size_t offset, size_t num) const
{
    mRenderData_.uiNumRenderedVertex += 4*data.size();

    for (auto& mFace : data)
    {
        color mColor = block::HUE_TABLE[mFace.hue];
        if (mFace.b == pSelectedBlock_)
            mColor = mColor*color::RED;

        float fOcclusion = lLightingArray_->get_pixel(
            mFace.sunlight/uiLightingArrayRatio_, mFace.light/uiLightingArrayRatio_
        ).a/255.0f;

        render_vertex_smooth_(mFace.v1, mColor, fOcclusion);
        render_vertex_smooth_(mFace.v2, mColor, fOcclusion);
        render_vertex_smooth_(mFace.v3, mColor, fOcclusion);
        render_vertex_smooth_(mFace.v4, mColor, fOcclusion);
    }
}

void world::render_faces_shader_smooth_(const std::vector<block_face>& data, size_t offset, size_t num) const
{
    mRenderData_.uiNumRenderedVertex += 4*data.size();

    for (auto& mFace : data)
    {
        color mColor = block::HUE_TABLE[mFace.hue];
        if (mFace.b == pSelectedBlock_)
            mColor = mColor*color::RED;

        glColor4fv(reinterpret_cast<float*>(&mColor));

        render_vertex_shader_smooth_(mFace.v1);
        render_vertex_shader_smooth_(mFace.v2);
        render_vertex_shader_smooth_(mFace.v3);
        render_vertex_shader_smooth_(mFace.v4);
    }
}

void world::render_vertex_(const vertex& v) const
{
    glTexCoord2fv(reinterpret_cast<const float*>(&v.uv));
    glVertex3fv(reinterpret_cast<const float*>(&v.pos));
}

void world::render_vertex_shader_(const vertex& v, const block_face& mFace) const
{
    glTexCoord4f(v.uv.u, v.uv.v, mFace.sunlight/256.0f, mFace.light/256.0f);
    glVertex3fv(reinterpret_cast<const float*>(&v.pos));
}

void world::render_vertex_smooth_(const vertex& v, const color& mColor, const float& fOcclusion) const
{
    const texture::color& mLight = lLightingArray_->get_pixel(
        v.sunlight/uiLightingArrayRatio_, v.light/uiLightingArrayRatio_
    );

    if (v.occlusion == 0u)
    {
        glColor4f(
            mColor.r*mLight.r/255.0f,
            mColor.g*mLight.g/255.0f,
            mColor.b*mLight.b/255.0f,
            mColor.a
        );
    }
    else
    {
        float o = 1.0f + (block::OCCLUSION_TABLE1[v.occlusion]-1.0f)*fOcclusion;
        glColor4f(
            mColor.r*mLight.r/255.0f*o,
            mColor.g*mLight.g/255.0f*o,
            mColor.b*mLight.b/255.0f*o,
            mColor.a
        );
    }

    glTexCoord2fv(reinterpret_cast<const float*>(&v.uv));
    glVertex3fv(reinterpret_cast<const float*>(&v.pos));
}

void world::render_vertex_shader_smooth_(const vertex& v) const
{
    glMultiTexCoord2f(GL_TEXTURE0, v.uv.u, v.uv.v);
    glMultiTexCoord3f(GL_TEXTURE1, v.sunlight/256.0f, v.light/256.0f, v.occlusion/256.0f);
    glVertex3fv(reinterpret_cast<const float*>(&v.pos));
}

void world::render_selected_block_() const
{
    if (!pSelectedBlock_)
        return;

    std::shared_ptr<const block_chunk> pChunk = pSelectedChunk_.lock();
    const block* pNextBlock = pChunk->get_block(mSelectedFace_, pSelectedBlock_).second;
    if (!pNextBlock)
        return;

    vector3f mBlockPos = vector3f(pChunk->get_block_position(pSelectedBlock_));
    vector3f mPosition = pChunk->get_position(pCurrentChunk_);
    if (bUseVBO_)
    {
        glPushMatrix();
        glTranslatef(mPosition.x, mPosition.y, mPosition.z);
    }
    else
        mBlockPos += mPosition;

    std::vector<block_face> lArray;
    pSelectedBlock_->add_face(
        mBlockPos, mSelectedFace_, pNextBlock, (uchar)255u, block::TEX_SELECTED, lArray
    );

    block::BLOCK_SELECTED_TEXTURE->bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(0.0f, -2);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);

    render_faces_(lArray, 0, 1);

    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    if (bUseVBO_)
        glPopMatrix();
}

