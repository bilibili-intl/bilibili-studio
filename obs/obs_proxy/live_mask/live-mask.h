#pragma once

#include <vector>
#include <stdint.h>

class AlphaToSvgBin
{
public:
    virtual ~AlphaToSvgBin() {}
    virtual bool AppendMask(uint8_t* data, int word_bytes, int width, int height, int stride, float(*transform)[4] = 0) = 0;
    virtual std::vector<uint8_t> GetSEI(bool isAnnexB) = 0;
};

AlphaToSvgBin* CreatePotraceAlphaToSvgBin(int sceneWidth, int sceneHeight);
