#include "../Source/LoopViewHistoryV027.h"

#include <cassert>
#include <iostream>
#include <limits>

using liberty::loop_v027::View;
using liberty::loop_v027::ViewHistory;
using liberty::loop_v027::valid;
using liberty::loop_v027::same;

int main()
{
    ViewHistory history;
    View restored {};
    const View original {10000, 3000, 2000, 5.0};
    const View zoomed {10000, 3500, 1000, 10.0};
    assert(valid(original) && valid(zoomed));
    assert(!history.available());
    assert(!history.toggle(original, restored));
    assert(history.remember(original));
    assert(history.available());
    assert(history.toggle(zoomed, restored));
    assert(same(restored, original));
    // A second Shift-double-click returns to the other view.
    assert(history.toggle(original, restored));
    assert(same(restored, zoomed));

    // Every new navigation replaces the one-step snapshot.
    const View panned {10000, 4000, 1000, 10.0};
    assert(history.remember(zoomed));
    assert(history.toggle(panned, restored));
    assert(same(restored, zoomed));

    // A repeat of the identical viewport cannot lose previous history.
    assert(!history.toggle(panned, restored));
    history.clear();
    assert(!history.available());
    assert(!history.toggle(zoomed, restored));

    // Save only valid viewpoints; invalid saves do not corrupt state.
    assert(history.remember(original));
    assert(!history.remember({10000, -1, 1000, 10.0}));
    assert(!history.remember({10000, 9000, 2000, 10.0}));
    assert(!history.remember({10000, 3000, 0, 10.0}));
    assert(!history.remember({10000, 3000, 2000, 0.0}));
    assert(!history.remember({10000, 3000, 2000,
                              std::numeric_limits<double>::infinity()}));
    assert(!history.remember({10000, 3000, 2000,
                              std::numeric_limits<double>::quiet_NaN()}));
    assert(history.toggle(zoomed, restored));
    assert(same(restored, original));

    // A new sample (even with an identically positioned viewport) must
    // never restore a view from the previous WAV.
    history.clear();
    assert(history.remember({10000, 0, 10000, 1.0}));
    assert(!history.toggle({20000, 0, 20000, 1.0}, restored));

    constexpr int limit = std::numeric_limits<int>::max();
    history.clear();
    const View veryLong {limit, limit - 1000, 1000, 214748.0};
    assert(valid(veryLong) && history.remember(veryLong));
    assert(history.toggle({limit, limit - 2000, 2000, 107374.0}, restored));
    assert(same(restored, veryLong));
    assert(!valid({limit, limit - 999, 1000, 1.0}));

    const View smallest {1, 0, 1, 1.0};
    assert(valid(smallest));
    history.clear();
    assert(history.remember(smallest));
    assert(!history.toggle(smallest, restored));
    std::cout << "v0.27 viewport history: undo, redo, WAV boundaries OK\n";
}
