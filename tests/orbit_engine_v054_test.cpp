#include "../Source/OrbitEngineV054.h"
#include "../Source/MorphWeightsV036.h"
#include <cassert>
#include <cmath>
#include <iostream>
using liberty::orbit_v054::Engine;
using liberty::orbit_v054::Point;
static bool same(float a,float b,float eps=1.e-5f) {return std::abs(a-b)<eps;}
int main() {
    Engine e;
    const Point centre{.31f,.67f};
    for(int m=-5;m<=0;++m) {
        const auto p=e.next(512,44100,m,1.f,1.f,11,centre,true);
        assert(p.x==centre.x && p.y==centre.y);
    }
    for(int mode=1;mode<=3;++mode) {
        const auto p=e.next(1024,48000,mode,.5f,0.f,7,centre,false);
        assert(p.x==centre.x && p.y==centre.y);
        const auto w=liberty::morph_v036::weights(p.x,p.y);
        const auto old=liberty::morph_v036::weights(centre.x,centre.y);
        for(int i=0;i<4;++i) assert(w[i]==old[i]);
    }
    for(int mode=1;mode<=3;++mode) {
        e.reset();
        const auto a=e.next(512,48000,mode,1.f,1.f,7,centre,false);
        const auto b=e.next(24000,48000,mode,1.f,1.f,7,centre,false);
        assert(!same(a.x,b.x) || !same(a.y,b.y));
        const auto weights=liberty::morph_v036::weights(b.x,b.y);
        float sum=0.f;
        for(float x:weights){assert(std::isfinite(x)&&x>=0.f&&x<=1.f);sum+=x;}
        assert(same(sum,1.f));
    }
    Engine first,second;
    for(int i=0;i<42;++i) {
        const auto a=first.next(257,44100,3,.48f,.75f,4,centre,false);
        const auto b=second.next(257,44100,3,.48f,.75f,4,centre,false);
        assert(a.x==b.x && a.y==b.y);
    }
    assert(liberty::orbit_v054::constellation(3,0).x !=
           liberty::orbit_v054::constellation(4,0).x);
    first.reset();second.reset();
    const auto ung=first.next(512,48000,2,.25f,1.f,3,centre,true);
    second.noteOn(127,1.f);
    const auto grav=second.next(512,48000,2,.25f,1.f,3,centre,true);
    assert(grav.x>ung.x && grav.y>ung.y);
    second.noteOff(127);
    const auto after=second.next(512,48000,2,.25f,1.f,3,centre,true);
    const auto ungravLater=first.next(512,48000,2,.25f,1.f,3,centre,true);
    assert(after.x==ungravLater.x && after.y==ungravLater.y);
    second.noteOn(0,.5f);second.noteOn(60,.8f);second.noteOff(60);
    const auto low=second.next(512,48000,2,.25f,1.f,3,centre,true);
    assert(std::isfinite(low.x)&&std::isfinite(low.y));
    second.allNotesOff();
    for(int mode=0;mode<=3;++mode) {
        const auto p=second.next(65536,96000,mode,NAN,INFINITY,20,{-INFINITY,NAN},true);
        assert(p.x>=0&&p.x<=1&&p.y>=0&&p.y<=1);
    }
    std::cout<<"Liberty ORBIT v0.54 deterministic trajectories + MIDI gravity PASS\n";
}
