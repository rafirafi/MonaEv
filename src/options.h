#ifndef OPTIONS_H
#define OPTIONS_H

struct Options {
    struct Painter {
        std::string imagePath;
        bool dropNeutral;
        int stepSize;
    };
    struct Manager {
        //std::string dnaPath;
        std::string dna;
        int polygonCount;
        int vertexCount;
        int sliceCount;
    };
    struct Evaluator {
        bool useSSd;
        bool force16Bit;
    };
    Painter painter;
    Manager manager;
    Evaluator evaluator;
};

#endif // OPTIONS_H
