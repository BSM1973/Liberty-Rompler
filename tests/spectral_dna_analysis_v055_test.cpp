#include "../Source/SpectralDNAAnalysisV055.h"
#include <array>
#include <cassert>
#include <cmath>
#include <limits>
using namespace liberty::spectral_dna_v055;
int main() {
    constexpr float tau=6.2831853071795864769f;
    std::array<float,kFftSize> low{},high{},silence{};
    for(std::size_t i=0;i<kFftSize;++i) {
        low[i]=std::sin(tau*8.f*static_cast<float>(i)/static_cast<float>(kFftSize));
        high[i]=std::sin(tau*24.f*static_cast<float>(i)/static_cast<float>(kFftSize));
    }
    const auto a=analyse(low.data(),low.size());
    const auto b=analyse(high.data(),high.size());
    assert(a[8]>0.99f && a[8]>a[24]*20.f);
    assert(b[24]>0.99f && b[24]>b[8]*20.f);
    assert(analyse(nullptr,0)[0]==0.f);
    assert(analyse(silence.data(),silence.size())[8]==0.f);
    low[0]=std::numeric_limits<float>::quiet_NaN();
    low[1]=std::numeric_limits<float>::infinity();
    const auto safe=analyse(low.data(),low.size());
    for(float v:safe) assert(std::isfinite(v) && v>=0.f && v<=1.f);
    std::array<const float*,4> windows{{low.data(),high.data(),nullptr,silence.data()}};
    std::array<std::size_t,4> sizes{{kFftSize,kFftSize,0,kFftSize}};
    const auto four=analyseFour(windows,sizes);
    assert(four[0][8]>0.9f && four[1][24]>0.99f && four[2][0]==0.f);
    Settings settings;
    settings.enabled=true;settings.wet=0.75f;settings.x=0.f;settings.y=0.f;
    const auto left=calculate(four,settings,0.f);
    assert(left.target[8]>0.9f && left.wet>0.74f);
    settings.x=1.f;settings.y=0.f;
    const auto right=calculate(four,settings,1.f);
    assert(right.target[24]>0.99f && right.wet==0.f);
}
