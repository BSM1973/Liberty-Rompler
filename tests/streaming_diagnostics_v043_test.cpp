#include "../Source/StreamingDiagnosticsV043.h"
#include <cassert>
#include <cmath>
#include <iostream>
int main() {
    using liberty::stream_v042::HealthSnapshot;
    liberty::stream_v043::Diagnostics d;
    assert(!d.hasStreamedZones());
    assert(!d.hasMeasurements());
    assert(!d.hasIncidents());
    HealthSnapshot a;
    a.cacheHits = 90;
    a.cacheMisses = 10;
    a.publishedPages = 7;
    a.urgentQueued = 2;
    a.urgentServiced = 1;
    d.add(a);
    assert(d.streamedZones == 1 && d.hasMeasurements());
    assert(std::abs(d.hitRatePercent() - 90.0) < 1.0e-8);
    HealthSnapshot b;
    b.cacheHits = 10;
    b.cacheMisses = 90;
    b.droppedPages = 1;
    b.urgentDropped = 2;
    b.decoderErrors = 3;
    b.underflowBlocks = 4;
    d.add(b);
    assert(d.streamedZones == 2);
    assert(d.totals.cacheHits == 100 && d.totals.cacheMisses == 100);
    assert(std::abs(d.hitRatePercent() - 50.0) < 1.0e-8);
    assert(d.totals.droppedPages == 1 && d.totals.urgentDropped == 2);
    assert(d.totals.decoderErrors == 3 && d.totals.underflowBlocks == 4);
    assert(d.hasIncidents());
    std::cout << "v0.43 aggregated diagnostics OK\n";
}
