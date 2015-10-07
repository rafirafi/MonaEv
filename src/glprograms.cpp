#include "glprograms.h"

const char * const colorDrawerVert = R"glsl(
    #version 120

    uniform mat4 mvp;
    attribute vec2 coord2d;
    attribute vec4 acolor;
    varying vec4 excolor;

    void main()
    {
        excolor = acolor;
        gl_Position = mvp * vec4(coord2d, 0.0, 1.0);
    }
)glsl";

const char * const colorDrawerFrag = R"glsl(
    #version 120

    varying vec4 excolor;

    void main()
    {
        gl_FragColor = excolor;
    }
)glsl";

const char * const texDrawerVert = R"glsl(
    #version 120

    uniform mat4 mvp;
    attribute vec2 coord2d;
    varying vec2 texCoord;

    void main()
    {
        texCoord = coord2d;
        gl_Position = mvp * vec4(coord2d, 0.0, 1.0);
    }
)glsl";

const char * const texDrawerFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    void main()
    {
        gl_FragColor = texture2D(texUnit0, (texCoord / texSize));
    }
)glsl";

const char * const texDrawerRevYFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    void main()
    {
        vec2 coo = vec2(texCoord.x / texSize.x, (texSize.y - texCoord.y) / texSize.y);
        gl_FragColor = texture2D(texUnit0, coo);
    }
)glsl";

const char * const texNormalizeDrawerFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    void main()
    {
        vec4 value = floor((0.5 + (min (max (texture2D (texUnit0, (texCoord / texSize)), 0.0), 1.0) * 255.0)));
        gl_FragColor = vec4(value[0] + value[1] + value[2] + value[3], 0.0, 0.0, 0.0);
    }
)glsl";

    // TODO: glsl 'dot()'
const char * const texMaxAbsDiffFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    void main()
    {
        vec4 value = floor((0.5 + (min (max (texture2D (texUnit0, (texCoord / texSize)), 0.0), 1.0) * 255.0)));
        value = max(value, vec4(255.0, 255.0, 255.0, 255.0) - value);
        gl_FragColor = vec4(value[0] + value[1] + value[2] + value[3], 0.0, 0.0, 0.0);
    }
)glsl";

const char * const texMaxSquaredDiffFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    void main()
    {
        vec4 value = floor((0.5 + (min (max (texture2D (texUnit0, (texCoord / texSize)), 0.0), 1.0) * 255.0)));
        value = max(value, vec4(255.0, 255.0, 255.0, 255.0) - value);
        gl_FragColor = vec4(value[0] * value[0] + value[1] * value[1]
                + value[2] * value[2] + value[3] * value[3], 0.0, 0.0, 0.0);
    }
)glsl";

const char * const tex16MaxSquaredDiffFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    vec4 normalize16(vec4 val)
    {
        if (val[0] > 2048.0) {
            float cnt = floor(val[0] / 2048.0);
            val[0] = val[0] - (cnt * 2048.0);
            val[1] = val[1] + cnt;
        }
        if (val[1] > 2048.0) {
            float cnt = floor(val[1] / 2048.0);
            val[1] = val[1] - (cnt * 2048.0);
            val[2] = val[2] + cnt;
        }
        if (val[2] > 2048.0) {
            float cnt = floor(val[2] / 2048.0);
            val[2] = val[2] - (cnt * 2048.0);
            val[3] = val[3] + cnt;
        }
        return val;
    }

    void main()
    {
        vec4 value = floor((0.5 + (min (max (texture2D (texUnit0, (texCoord / texSize)), 0.0), 1.0) * 255.0)));
        value = max(value, vec4(255.0, 255.0, 255.0, 255.0) - value);

        vec4 result = vec4(value[0] * value[0] + value[1] * value[1]
                + value[2] * value[2] + value[3] * value[3], 0.0, 0.0, 0.0);
        gl_FragColor = normalize16(result);
    }
)glsl";

const char * const texStoreFrag = R"glsl(
    #version 120

    uniform vec2 texSizeLimit; // used to store origin
    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    void main ()
    {
      gl_FragColor = texture2D (texUnit0, ((texCoord - texSizeLimit) / texSize));
    }
)glsl";

const char * const texDiffVert = R"glsl(
    #version 120

    uniform mat4 mvp;
    attribute vec2 coord2d;
    varying vec2 texCoord;

    void main()
    {
        texCoord = coord2d;
        gl_Position = mvp * vec4(coord2d, 0.0, 1.0);
    }
)glsl";

const char * const texAbsDiffFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    uniform sampler2D texUnit1;
    varying vec2 texCoord;

    vec4 toUint8(const vec4 a)
    {
        return floor(0.5 + min(max(a, 0.0), 1.0) * 255.0);
    }

    void main()
    {
        vec4 result = abs(toUint8(texture2D(texUnit0, texCoord / texSize))
                          - toUint8(texture2D(texUnit1, texCoord / texSize)));

        gl_FragColor = vec4(result[0] + result[1] + result[2] + result[3], 0.0, 0.0, 0.0);
    }
)glsl";

const char * const texSquaredDiffFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    uniform sampler2D texUnit1;
    varying vec2 texCoord;

    vec4 toUint8(const vec4 a)
    {
        return floor(0.5 + min(max(a, 0.0), 1.0) * 255.0);
    }

    void main()
    {
        vec4 result = abs(toUint8(texture2D(texUnit0, texCoord / texSize))
                          - toUint8(texture2D(texUnit1, texCoord / texSize)));

       gl_FragColor = vec4(result[0] * result[0] + result[1] * result[1]
               + result[2] * result[2] + result[3] * result[3], 0.0, 0.0, 0.0);
    }
)glsl";

const char * const tex16SquaredDiffFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform sampler2D texUnit0;
    uniform sampler2D texUnit1;
    varying vec2 texCoord;

    vec4 toUint8(const vec4 a)
    {
        return floor(0.5 + min(max(a, 0.0), 1.0) * 255.0);
    }

    vec4 normalize16(vec4 val)
    {
        if (val[0] > 2048.0) {
            float cnt = floor(val[0] / 2048.0);
            val[0] = val[0] - (cnt * 2048.0);
            val[1] = val[1] + cnt;
        }
        if (val[1] > 2048.0) {
            float cnt = floor(val[1] / 2048.0);
            val[1] = val[1] - (cnt * 2048.0);
            val[2] = val[2] + cnt;
        }
        if (val[2] > 2048.0) {
            float cnt = floor(val[2] / 2048.0);
            val[2] = val[2] - (cnt * 2048.0);
            val[3] = val[3] + cnt;
        }
        return val;
    }

    void main()
    {
        vec4 value = abs(toUint8(texture2D(texUnit0, texCoord / texSize))
                          - toUint8(texture2D(texUnit1, texCoord / texSize)));

        vec4 result = vec4(value[0] * value[0] + value[1] * value[1]
                + value[2] * value[2] + value[3] * value[3], 0.0, 0.0, 0.0);
        gl_FragColor = normalize16(result);
    }
)glsl";

const char * const texSumComputerVert = R"glsl(
    #version 120

    uniform mat4 mvp;
    attribute vec2 coord2d;
    varying vec2 texCoord;

    void main()
    {
        texCoord = coord2d;
        gl_Position = mvp * vec4(coord2d, 0.0, 1.0);
    }
)glsl";

const char * const tex32SumComputerFrag = R"glsl(
    #version 120

    uniform vec2 texSize;
    uniform vec2 texSizeLimit;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    void main()
    {
        vec2 cooBase = (floor(texCoord) * vec2(2.0, 2.0)) + vec2(0.5, 0.5);

        vec2 coos[4];
        coos[0] = cooBase;
        coos[1] = cooBase + vec2(1.0, 0.0);
        coos[2] = cooBase + vec2(0.0, 1.0);
        coos[3] = cooBase + vec2(1.0, 1.0);

        float sum = 0.0;

        if ((coos[0][0] < texSizeLimit[0]) && (coos[0][1] < texSizeLimit[1])) {
            sum = sum + texture2D(texUnit0, coos[0] / texSize)[0];
        }

        if ((coos[1][0] < texSizeLimit[0]) && (coos[1][1] < texSizeLimit[1])) {
            sum = sum + texture2D(texUnit0, coos[1] / texSize)[0];
        }

        if ((coos[2][0] < texSizeLimit[0]) && (coos[2][1] < texSizeLimit[1])) {
            sum = sum + texture2D(texUnit0, coos[2] / texSize)[0];
        }

        if ((coos[3][0] < texSizeLimit[0]) && (coos[3][1] < texSizeLimit[1])) {
            sum = sum + texture2D(texUnit0, coos[3] / texSize)[0];
        }

        gl_FragColor = vec4(sum, 0.0, 0.0, 0.0);
    }
)glsl";

const char * const tex16SumComputerFrag = R"glsl(
    #version 120

    uniform vec2 texSize; // size of the sampled texture
    uniform vec2 texSizeLimit;
    uniform sampler2D texUnit0;
    varying vec2 texCoord;

    vec4 doSum(vec2 coo, vec4 sum)
    {
        // intel gm45 on windows OS is unable to unloop correctly
        sum[0] = sum[0] + texture2D(texUnit0, coo)[0];
        if (sum[0] > 2048.0) {
            float cnt = floor(sum[0] / 2048.0);
            sum[0] = sum[0] - (cnt * 2048.0);
            sum[1] = sum[1] + cnt;
        }
        sum[1] = sum[1] + texture2D(texUnit0, coo)[1];
        if (sum[1] > 2048.0) {
            float cnt = floor(sum[1] / 2048.0);
            sum[1] = sum[1] - (cnt * 2048.0);
            sum[2] = sum[2] + cnt;
        }
        sum[2] = sum[2] + texture2D(texUnit0, coo)[2];
        if (sum[2] > 2048.0) {
            float cnt = floor(sum[2] / 2048.0);
            sum[2] = sum[2] - (cnt * 2048.0);
            sum[3] = sum[3] + cnt;
        }
        sum[3] = sum[3] + texture2D(texUnit0, coo)[3];

        return sum;
    }

    void main()
    {
        // half float 0-2048 exactly represented        
        vec2 cooBase = (floor(texCoord) * vec2(2.0, 2.0)) + vec2(0.5, 0.5);

        vec2 coos[4];
        coos[0] = cooBase;
        coos[1] = cooBase + vec2(1.0, 0.0);
        coos[2] = cooBase + vec2(0.0, 1.0);
        coos[3] = cooBase + vec2(1.0, 1.0);

        vec4 sum = vec4(0., 0., 0., 0.);

        if ((coos[0][0] < texSizeLimit[0]) && (coos[0][1] < texSizeLimit[1])) {
            sum = doSum(coos[0] / texSize, sum);
        }

        if ((coos[1][0] < texSizeLimit[0]) && (coos[1][1] < texSizeLimit[1])) {
            sum = doSum(coos[1] / texSize, sum);
        }

        if ((coos[2][0] < texSizeLimit[0]) && (coos[2][1] < texSizeLimit[1])) {
            sum = doSum(coos[2] / texSize, sum);
        }

        if ((coos[3][0] < texSizeLimit[0]) && (coos[3][1] < texSizeLimit[1])) {
            sum = doSum(coos[3] / texSize, sum);
        }

        gl_FragColor = sum;
    }
)glsl";
