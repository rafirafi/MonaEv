#ifndef GLSLPROGRAMS_H
#define GLSLPROGRAMS_H

/* '16' variants have integer packed in vec4 in base 2048 */

extern const char * const colorDrawerVert;
extern const char * const colorDrawerFrag;

extern const char * const texDrawerVert;
extern const char * const texDrawerFrag;
extern const char * const texDrawerRevYFrag;
extern const char * const texNormalizeDrawerFrag;
extern const char * const texMaxAbsDiffFrag;
extern const char * const texMaxSquaredDiffFrag;
extern const char * const tex16MaxSquaredDiffFrag;
extern const char * const texStoreFrag;

extern const char * const texDiffVert;
extern const char * const texAbsDiffFrag;
extern const char * const texSquaredDiffFrag;
extern const char * const tex16SquaredDiffFrag;

extern const char * const texSumComputerVert;
extern const char * const tex16SumComputerFrag;
extern const char * const tex32SumComputerFrag;

#endif // GLSLPROGRAMS_H
