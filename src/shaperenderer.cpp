#include "shaperenderer.h"

#include <tuple>

#include "glprograms.h"

ShapeRenderer::~ShapeRenderer()
{
    for (auto &slice : shapeSlices_) {
        slice.free();
    }

    glDeleteFramebuffers(fbos_.size(), fbos_.data());
    glDeleteTextures(texComposed_.size(), texComposed_.data());
    glDeleteTextures(texPlanes_.size(), texPlanes_.data());
}

bool ShapeRenderer::init(const ShapeCollection &collection, int sliceNbTarget)
{
    bool result = true;

    if (sliceNbTarget) {
        sliceNbTarget_ = sliceNbTarget;
    }
    shapesPerSliceMax_ = collection.polygonCount / sliceNbTarget_;
    shapesPerSliceMax_ += ((collection.polygonCount % sliceNbTarget_) != 0);

    shapeSlicesSize_ = collection.polygonCount / shapesPerSliceMax_;
    shapeSlicesSize_ += (collection.polygonCount % shapesPerSliceMax_ != 0);

    shapeSlices_.resize(shapeSlicesSize_);
    for (int i = 0, off = 0; off < collection.polygonCount; off += shapesPerSliceMax_, i++) {
        int size = (off + shapesPerSliceMax_ > collection.polygonCount ?
                        collection.polygonCount - off : shapesPerSliceMax_);

        shapeSlices_[i].init(tObj_, collection , off, off + size - 1);
    }

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &fboAttachmentCount_);
    fboAttachmentCount_ = (fboAttachmentCount_ * 2) / 2;
    int fboNbNeeded = (2 * shapeSlicesSize_) / fboAttachmentCount_ +
            (((2 * shapeSlicesSize_) % fboAttachmentCount_) != 0);

    fbos_.resize(fboNbNeeded, 0);
    texPlanes_.resize(shapeSlicesSize_, 0);
    texComposed_.resize(shapeSlicesSize_, 0);
    planesValid_.resize(shapeSlicesSize_, false);

    for (int fboIdx = 0, sliceIdx = 0; fboIdx < fboNbNeeded; fboIdx++) {
        int texturesSize = fboAttachmentCount_;
        if (fboIdx + 1 == fboNbNeeded && (2 * shapeSlicesSize_) % fboAttachmentCount_ != 0) {
            texturesSize = (2 * shapeSlicesSize_) % fboAttachmentCount_;
        }
        std::vector<GLuint> textures(texturesSize);

        fbos_[fboIdx] = prepareFbo(collection.width,
                                   collection.height,
                                   GL_RGBA8,
                                   GL_RGBA,
                                   GL_UNSIGNED_BYTE,
                                   textures.data(),
                                   texturesSize);

        if (fbos_[fboIdx] == 0) { // TODO : error path
            result = false;
            break;
        }

        for (int i = 0; i < texturesSize; i += 2, sliceIdx++) {
            texPlanes_[sliceIdx] = textures[i];
            texComposed_[sliceIdx] = textures[i + 1];
        }
    }

    colorDrawerProg_.init(colorDrawerVert, colorDrawerFrag, collection.width, -collection.height);
    texDrawerProg_.init(texDrawerVert, texDrawerFrag, collection.width, collection.height);
    glUseProgram(texDrawerProg_.programId());
    texDrawerProg_.setTextureSize(collection.width, collection.height);
    texDrawerProg_.setTextureUnit(0);
    glUseProgram(0);

    quadDrawer_.init(collection.width, collection.height);

    return result;
}

GLuint ShapeRenderer::render()
{
    int fboIdx, texturePlaneIdx;
    int fboBound = -1;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_BLEND);

    // render slices
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
    glUseProgram(colorDrawerProg_.programId());
    int compPlaneStartIdx = shapeSlicesSize_;
    for (int sliceIdx = 0; sliceIdx < shapeSlicesSize_; sliceIdx++) {
        if (planesValid_[sliceIdx] == false) {
            if (compPlaneStartIdx == shapeSlicesSize_ && shapeSlicesSize_ != 1) {
                compPlaneStartIdx = sliceIdx;
            }
            std::tie(fboIdx, texturePlaneIdx) = getSliceFboTextureIdx(sliceIdx);
            if (fboBound != fboIdx) {
                glBindFramebuffer(GL_FRAMEBUFFER, fbos_[fboIdx]);
                fboBound = fboIdx;
            }

            GLenum bufId = GL_COLOR_ATTACHMENT0 + texturePlaneIdx;
            glDrawBuffer(bufId);
            glClear(GL_COLOR_BUFFER_BIT);
            shapeSlices_[sliceIdx].render();

            planesValid_[sliceIdx] = true;
        }
    }

    // composition
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(texDrawerProg_.programId());
    for (int sliceIdx = compPlaneStartIdx; sliceIdx < shapeSlicesSize_; sliceIdx++) {

        std::tie(fboIdx, texturePlaneIdx) = getSliceFboTextureIdx(sliceIdx);
        int textureCompIdx = texturePlaneIdx + 1;

        if (fboBound != fboIdx) {
            glBindFramebuffer(GL_FRAMEBUFFER, fbos_[fboIdx]);
            fboBound = fboIdx;
        }

        glDrawBuffer(GL_COLOR_ATTACHMENT0 + textureCompIdx);

        // prepare composition
        if (sliceIdx == 0) {
            glClear(GL_COLOR_BUFFER_BIT);
        } else {
            glDisable(GL_BLEND);
            glActiveTexture(GL_TEXTURE0);
            GLuint prevCompTexture = texComposed_[sliceIdx - 1];
            glBindTexture(GL_TEXTURE_2D, prevCompTexture);
            quadDrawer_.draw();
            glEnable(GL_BLEND);
        }

        // do composition
        GLuint renderedTexture = texPlanes_[sliceIdx];
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        quadDrawer_.draw();
    }

    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);

    return (shapeSlicesSize_ == 1 ? texPlanes_[0] : texComposed_[shapeSlicesSize_ - 1]);
}

void ShapeRenderer::update(const Mutation &mutation, const ShapeCollection &collection)
{
    int sliceId = mutation.shapeIdx / shapesPerSliceMax_;
    planesValid_[sliceId] = false;
    shapeSlices_[sliceId].update(collection, mutation.shapeIdx, tObj_);

    if (mutation.type == Mutation::kOrderSwapped) {
        sliceId = mutation.data.shapeIdxB / shapesPerSliceMax_;
        planesValid_[sliceId] = false;
        shapeSlices_[sliceId].update(collection, mutation.data.shapeIdxB, tObj_);
    }
}

std::pair<int, int> ShapeRenderer::getSliceFboTextureIdx(int sliceIdx) const
{
    return { (2 * sliceIdx) / fboAttachmentCount_, (2 * sliceIdx) % fboAttachmentCount_ };
}
