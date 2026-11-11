#include "../Source/MultiSamplePolicyV037.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace liberty::multisample_v037;
static bool nearly(float a, float b) { return std::abs(a-b)<0.00001f; }

int main()
{
    assert(nearly(velocityWeight(20,1,127,0),1.0f));
    assert(nearly(velocityWeight(20,30,80,0),0.0f));
    assert(nearly(velocityWeight(30,30,80,0),1.0f));
    assert(nearly(velocityWeight(80,30,80,0),1.0f));
    assert(nearly(velocityWeight(81,30,80,0),0.0f));
    assert(nearly(velocityWeight(127,1,127,8),1.0f));
    assert(nearly(velocityWeight(1,1,127,8),1.0f));
    assert(velocityWeight(62,1,63,8)>0.0f);
    assert(velocityWeight(64,64,127,8)>0.0f);
    // Adjacent bands sum to unity through the transition.
    for(int vel=40;vel<88;++vel)
       assert(nearly(velocityWeight(vel,1,63,8)+
                     velocityWeight(vel,64,127,8),1.0f));
    assert(nearly(velocityWeight(100,1,63,8),0.0f));
    assert(nearly(velocityWeight(0,1,127,8),0.0f));
    assert(nearly(velocityWeight(100,90,80,8),0.0f));
    assert(nearly(velocityWeight(100,1,127,33),0.0f));

    assert(articulationAllowed(0,16));
    assert(articulationAllowed(2,2));
    assert(!articulationAllowed(2,1));
    RoundRobin rr;
    const auto takes = (1u<<1)|(1u<<2)|(1u<<4);
    assert(rr.select(60,1,takes)==1);
    assert(rr.select(60,1,takes)==2);
    assert(rr.select(60,1,takes)==4);
    assert(rr.select(60,1,takes)==1);
    assert(rr.select(60,2,takes)==1); // separate group
    assert(rr.select(61,1,takes)==1); // separate note
    assert(rr.select(60,0,takes)==0);
    assert(rr.select(60,17,takes)==0);
    assert(rr.select(60,1,0u)==0);
    rr.reset();
    assert(rr.select(60,1,takes)==1);
    assert(rr.select(60,1,(1u<<3))==3); // missing take is skipped
    std::cout<<"v0.37 RR, articulation and velocity transitions OK\n";
    return 0;
}
