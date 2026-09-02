#pragma once

#include <cstdint>

#include "types.hpp"

namespace pixel
{

    enum class ComposeOp
    {
        SrcOver,
        SrcOut
    };

    inline RGBA Composite_SrcOver(RGBA dst, RGBA src) {
        float srcR = src[0] / 255.0f;
        float srcG = src[1] / 255.0f;
        float srcB = src[2] / 255.0f;
        float srcA = src[3] / 255.0f;

        float dstR = dst[0] / 255.0f;
        float dstG = dst[1] / 255.0f;
        float dstB = dst[2] / 255.0f;
        float dstA = dst[3] / 255.0f;

        float ma = (1.0f - srcA); // minus alpha

        float oa = (srcA + dstA * ma); // out alpha
        if (oa <= 0.0f) {
            return C_BLANK;
        }
        float inv_oa = 1.0f / oa;

        return RGBA(
            (uint8_t)(((srcA * srcR + dstA * dstR * ma) * inv_oa) * 255.0f + 0.5f),
            (uint8_t)(((srcA * srcG + dstA * dstG * ma) * inv_oa) * 255.0f + 0.5f),
            (uint8_t)(((srcA * srcB + dstA * dstB * ma) * inv_oa) * 255.0f + 0.5f),
            (uint8_t)(oa * 255.0f + 0.5f)
        );
    }

    inline RGBA Composite_SrcOut(RGBA dst, RGBA src) {
        float srcR = src[0] / 255.0f;
        float srcG = src[1] / 255.0f;
        float srcB = src[2] / 255.0f;
        float srcA = src[3] / 255.0f;

        float dstR = dst[0] / 255.0f;
        float dstG = dst[1] / 255.0f;
        float dstB = dst[2] / 255.0f;
        float dstA = dst[3] / 255.0f;

        float fa = (1.0f - dstA);

        return RGBA(
            (uint8_t)((srcA * srcR * fa) * 255.0f + 0.5f),
            (uint8_t)((srcA * srcG * fa) * 255.0f + 0.5f),
            (uint8_t)((srcA * srcB * fa) * 255.0f + 0.5f),
            (uint8_t)((srcA * fa) * 255.0f + 0.5f)
        );
    }

    // inline void Composite_Atop(uint8_t* dst, RGBA src) {
    //     float src_f[4];
    //     src.ToFloat4(src_f);
        
    //     float as = src_f[3];
    //     float ab = dst[3] / 255.0f;
    //     float ma = (1.0f - as); // minus alpha

    //     dst[0] = (as * src_f[0] * ab + ab * dst[0] * ma) * 255.0f;
    //     dst[1] = (as * src_f[1] * ab + ab * dst[1] * ma) * 255.0f;
    //     dst[2] = (as * src_f[2] * ab + ab * dst[2] * ma) * 255.0f;
    //     dst[3] = (as * ab + ab * ma) * 255.0f;
    // }

    inline RGBA MakeCompose(RGBA dst, RGBA src, ComposeOp comp) {
        switch (comp) {
            case ComposeOp::SrcOver: return Composite_SrcOver(dst, src);
            case ComposeOp::SrcOut: return Composite_SrcOut(dst, src);
        }

        return dst;
    }

}