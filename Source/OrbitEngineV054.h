#pragma once
// Liberty Music Software - ORBIT v0.54: four-layer generative XY paths.
// Entire engine operates in fixed memory on the real-time audio thread.
// MANUEL mode is bit-identical to the pre-v0.54 MORPH coefficients.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace liberty::orbit_v054 {
struct Point { float x=0.5f, y=0.5f; };
inline float clamp01(float x) noexcept {
    return std::isfinite(x) ? std::clamp(x,0.0f,1.0f) : 0.5f;
}
inline std::uint32_t mix32(std::uint32_t x) noexcept {
    x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15;
    x *= 0x846ca68bU; x ^= x >> 16; return x;
}
inline Point constellation(int seed, int index) noexcept {
    const auto s=static_cast<std::uint32_t>(std::clamp(seed,0,15)+1);
    const auto i=static_cast<std::uint32_t>((index%8+8)%8+1);
    const auto a=mix32(s*0x9e3779b9U+i*0x85ebca6bU);
    const auto b=mix32(s*0xc2b2ae35U+i*0x27d4eb2fU);
    return { 0.1f+0.8f*(float)(a&65535U)/65535.0f,
             0.1f+0.8f*(float)(b&65535U)/65535.0f };
}
class Engine final {
public:
    void reset() noexcept { phase=0.0; velocity.fill(0.0f); mostRecent=-1; }
    void noteOn(int pitch, float strength) noexcept {
        if(pitch<0 || pitch>127) return;
        velocity[(std::size_t)pitch]=clamp01(strength);
        mostRecent=pitch;
    }
    void noteOff(int pitch) noexcept {
        if(pitch<0 || pitch>127) return;
        velocity[(std::size_t)pitch]=0.0f;
        if(mostRecent != pitch) return;
        mostRecent=-1;
        for(int n=127; n>=0; --n) if(velocity[(std::size_t)n]>0.0f) {
            mostRecent=n; break;
        }
    }
    void allNotesOff() noexcept { velocity.fill(0.0f); mostRecent=-1; }
    Point next(int blockFrames, double sampleRate, int mode,
               float cyclesPerSecond, float depth, int seed,
               Point centre, bool gravity) noexcept {
        centre.x=clamp01(centre.x); centre.y=clamp01(centre.y);
        if(mode < 1 || mode > 3) return centre;
        const float rate=std::isfinite(cyclesPerSecond)
            ? std::clamp(cyclesPerSecond,0.02f,2.0f) : 0.15f;
        const float amount=clamp01(depth);
        if(blockFrames>0 && std::isfinite(sampleRate) && sampleRate>=8000.0) {
            phase+=static_cast<double>(blockFrames)*static_cast<double>(rate)/sampleRate;
            phase-=std::floor(phase);
        }
        constexpr double tau=6.2831853071795864769;
        const double angle=tau*phase;
        Point shape{};
        if(mode==1) { // ORBIT : circular movement between four sonic layers.
            shape.x=0.5f+0.5f*(float)std::sin(angle);
            shape.y=0.5f-0.5f*(float)std::cos(angle);
        } else if(mode==2) { // INFINITY : continuous figure-eight in the XY plane.
            shape.x=0.5f+0.5f*(float)std::sin(angle);
            shape.y=0.5f+0.5f*(float)std::sin(2.0*angle);
        } else { // CONSTELLATION : seeded, repeatable eight-point flight.
            const double q=phase*8.0;
            const int n=(int)q;
            const float f=(float)(q-n);
            const float smooth=f*f*(3.0f-2.0f*f);
            const Point a=constellation(seed,n),b=constellation(seed,n+1);
            shape.x=a.x+(b.x-a.x)*smooth;
            shape.y=a.y+(b.y-a.y)*smooth;
        }
        Point out{clamp01(centre.x+amount*(shape.x-0.5f)),
                  clamp01(centre.y+amount*(shape.y-0.5f))};
        if(gravity && mostRecent>=0) {
            const float v=velocity[(std::size_t)mostRecent];
            const float attract=0.12f+0.60f*v;
            out.x=clamp01(out.x+(mostRecent/127.0f-out.x)*attract);
            out.y=clamp01(out.y+(v-out.y)*attract);
        }
        return out;
    }
private:
    double phase=0.0;
    std::array<float,128> velocity{};
    int mostRecent=-1;
};
} // namespace liberty::orbit_v054
