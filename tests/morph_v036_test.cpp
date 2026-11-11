#include "../Source/MorphWeightsV036.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::morph_v036::weights;
using liberty::morph_v036::fromMidi;

static bool near(float x, float y, float tolerance=0.00001f)
{
    return std::abs(x-y) < tolerance;
}

static void check(const std::array<float, 4>& w, float a,float b,float c,float d)
{
    assert(near(w[0],a) && near(w[1],b));
    assert(near(w[2],c) && near(w[3],d));
    assert(near(w[0]+w[1]+w[2]+w[3],1.0f));
    for(const float v : w) assert(v>=0.0f && v<=1.0f);
}

int main()
{
    check(weights(0.0f,0.0f),1.0f,0.0f,0.0f,0.0f); // A
    check(weights(1.0f,0.0f),0.0f,1.0f,0.0f,0.0f); // B
    check(weights(0.0f,1.0f),0.0f,0.0f,1.0f,0.0f); // C
    check(weights(1.0f,1.0f),0.0f,0.0f,0.0f,1.0f); // D
    check(weights(0.5f,0.5f),0.25f,0.25f,0.25f,0.25f);
    check(weights(0.25f,0.75f),0.1875f,0.0625f,0.5625f,0.1875f);
    check(weights(-40.0f,100.0f),0.0f,0.0f,1.0f,0.0f);
    check(weights(std::numeric_limits<float>::quiet_NaN(),0.0f),
          0.5f,0.5f,0.0f,0.0f);
    check(weights(1.0f,std::numeric_limits<float>::infinity()),
          0.0f,0.5f,0.0f,0.5f);

    assert(near(fromMidi(0),0.0f));
    assert(near(fromMidi(127),1.0f));
    assert(near(fromMidi(64),64.0f/127.0f));
    assert(near(fromMidi(-12),0.0f));
    assert(near(fromMidi(201),1.0f));
    for(int yi=0; yi<=127; yi+=13)
        for(int xi=0; xi<=127; xi+=11) {
            const auto w = weights(fromMidi(xi),fromMidi(yi));
            assert(near(w[0]+w[1]+w[2]+w[3],1.0f));
        }
    std::cout << "v0.36 MORPH weights, MIDI and boundaries OK\n";
    return 0;
}
