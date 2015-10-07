#include "shapemanager.h"

#include <stdio.h>

#include <chrono>

bool ShapeManager::init(int width, int height, int polygonCount, int vertexCount, int sliceNb)
{
    int alphaSeed = 127;
    collection_ = randomShapeCollection(width,
                                        height,
                                        polygonCount,
                                        vertexCount,
                                        alphaSeed);
    return initRenderer(sliceNb);
}

bool ShapeManager::init(int width, int height, const std::string &dna, int sliceNb)
{
    bool error = false;
    collection_ = fromDna(width, height, dna, error);
    if (error) {
        return false;
    }
    return initRenderer(sliceNb);
}

bool ShapeManager::initRenderer(int sliceNb)
{
    sliceNb = std::max(0, std::min(sliceNb, collection_.polygonCount));
    if (sliceNb == 0) {
        return renderer_.init(collection_, calibrate());
    }
    return renderer_.init(collection_, sliceNb);
}

int ShapeManager::calibrate()
{
    fprintf(stderr, "%s slice nb", __func__);

    int best = 1;

    int elapsedBase = 0;
    int elapsedBest = 0;

    std::array<int, 100> js;
    js.fill(collection_.polygonCount / 2);

    for (int i = 1; i < collection_.polygonCount; i++) {
        ShapeRenderer *renderer = new ShapeRenderer();
        renderer->init(collection_, i);
        renderer->render();

        fprintf(stderr, ".");
        auto start = std::chrono::high_resolution_clock::now();

        for (const auto &j : js) {
            Mutation mutation;
            mutation.type = Mutation::kColorChanged;
            mutation.shapeIdx = j;
            mutation.data.color = randomColor(-1);
            Mutation rmutation = setMutation(mutation, collection_);

            renderer->update(mutation, collection_);

            renderer->render();

            setMutation(rmutation, collection_);
            renderer->update(rmutation, collection_);

            glFlush();
        }

        glFinish();
        auto now = std::chrono::high_resolution_clock::now();
        int usElapsed = static_cast<int>(std::chrono::duration_cast
                                         <std::chrono::microseconds>(now - start).count());

        delete renderer;

        if (i == 1) {
            elapsedBase = usElapsed;
        } else if (i == 2) {
            elapsedBest = usElapsed;
        } else {
            if (elapsedBest < usElapsed) {
                if (i >= 25) {
                    break;
                }
            } else {
                elapsedBest = usElapsed;
                if (elapsedBest < elapsedBase) {
                    best = i;
                }
            }
        }

    }

    fprintf(stderr, " => %d\n", best);

    return best;
}

Mutation ShapeManager::mutate()
{
    mutation_ = getMutation(collection_);
    revMutation_ = setMutation(mutation_, collection_);

    renderer_.update(mutation_, collection_);

    return mutation_;
}

void ShapeManager::revMutate()
{
    applyMutation(revMutation_);
}

void ShapeManager::applyMutation(const Mutation &mutation)
{
    setMutation(mutation, collection_);

    renderer_.update(mutation, collection_);
}

GLuint ShapeManager::render()
{
    return renderer_.render();
}

std::string ShapeManager::getDna() const
{
    return ::getDna(collection_);
}

std::string ShapeManager::getSvg() const
{
    return ::getSvg(collection_);
}

std::array<int, 2> ShapeManager::getPolygonVertexCount() const
{
    return {{ collection_.polygonCount, collection_.vertexCount }};
}

Mutation ShapeManager::getRevMutation() const {
    return revMutation_;
}
