#include "../Source/LoopTransitionV050.h"
#include <cassert>
#include <cstdint>
#include <iostream>
int main() {
 using namespace liberty::stream_v050;
 const int length=24*44100, start=6*44100, end=(int)(19.2*44100);
 assert(plannedAttackFrames(length,44100,false)==66150);
 assert(plannedAttackFrames(length,44100,true)==143325);
 assert(plannedAttackFrames(length,192000,false)==147456);
 assert(plannedAttackFrames(length,0,false)==0);
 const auto w=planLoopWindow(length,start,end,66150,44100);
 assert(w.begin==start);
 assert(w.frames==33076);
 assert(w.frames<=maxLoopHeadFrames);
 assert(planLoopWindow(length,-1,end,66150,44100).frames==0);
 assert(planLoopWindow(length,start,length+1,66150,44100).frames==0);
 assert(planLoopWindow(length,start,start+10,66150,44100).frames==0);
 assert(planLoopWindow(length,1000,1100,66150,44100).frames==0);
 assert(planLoopWindow(length,start,end,66150,0.).frames==0);
 assert(planLoopWindow(2147483000,1000000,2147482000,60000,192000).frames==49152);
 // Four-layer / 64-zone bank: each loop head is bounded independently.
 const std::int64_t bytes=64LL*w.frames*2*sizeof(float);
 assert(bytes <= 64LL*maxLoopHeadFrames*2*static_cast<std::int64_t>(sizeof(float)));
 std::cout<<"v0.50 bounded poly64 attack and loop transitions OK\n";
}
