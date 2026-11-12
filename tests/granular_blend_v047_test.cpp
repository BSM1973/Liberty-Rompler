#include "../Source/GranularBlendV047.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
    using namespace liberty::granular_v047;
    assert(blend(.9f,-.5f,0.f)==.9f);
    assert(blend(.9f,-.5f,1.f)==-.5f);
    assert(std::abs(blend(.9f,-.5f,.5f)-.2f)<.00001f);
    assert(blend(.9f,-.5f,-1.f)==.9f);
    assert(blend(.9f,-.5f,2.f)==-.5f);
    const auto nan=std::nanf("");
    assert(blend(.9f,-.5f,nan)==.9f);
    float a=0.f;
    for(int i=0;i<500;++i) {
        auto next=nextWet(a,1.f);
        assert(next>=a && next<=1.f);
        a=next;
    }
    assert(a>.96f);
    for(int i=0;i<500;++i) a=nextWet(a,0.f);
    assert(a<.04f);
    std::cout<<"v0.47 granular dry/wet and smooth automation tests OK\n";
}
