#ifndef TESSPOLY2D_H
#define TESSPOLY2D_H

#include <vector>
#include <array>

struct TESStesselator;

class TessPoly2D
{
public:
    TessPoly2D();
    ~TessPoly2D();

    void tesselate(const std::vector<std::array<float, 2> > &polygon,
                   std::vector<std::array<float, 2> > *vertices,
                   std::vector<unsigned int> *indices = nullptr);

private:
    TESStesselator* tObj_ = nullptr;
};

#endif // TESSPOLY2D_H
