#pragma once
// Layout math for the sample-loop editor toolbar. JUCE-independent to allow
// deterministic overlap tests before a macOS build is requested.

#include <array>
#include <algorithm>

namespace liberty::loop_v015
{
struct Rect
{
    int x = 0, y = 0, width = 0, height = 0;
    bool visible() const noexcept { return width > 0 && height > 0; }
};

inline bool intersects (Rect a, Rect b) noexcept
{
    return a.visible() && b.visible() && a.x < b.x + b.width
           && b.x < a.x + a.width && a.y < b.y + b.height
           && b.y < a.y + a.height;
}

struct LoopToolbarLayout
{
    Rect beginLabel, beginMinus, beginFrame, beginPlus;
    Rect endLabel, endMinus, endFrame, endPlus;
    Rect preview, apply, cancel;
    bool fits = false;
    int requiredHeight = 0;

    std::array<Rect, 11> all() const noexcept
    {
        return {beginLabel, beginMinus, beginFrame, beginPlus,
                endLabel, endMinus, endFrame, endPlus,
                preview, apply, cancel};
    }

    bool noOverlap (int canvasWidth, int canvasHeight) const noexcept
    {
        const auto rects = all();
        for (std::size_t i = 0; i < rects.size(); ++i)
        {
            const auto& a = rects[i];
            if (! a.visible()) continue;
            if (a.x < 0 || a.y < 0 || a.x + a.width > canvasWidth
                || a.y + a.height > canvasHeight)
                return false;
            for (std::size_t j = i + 1; j < rects.size(); ++j)
                if (intersects (a, rects[j]))
                    return false;
        }
        return true;
    }
};

// At 740 px and up everything fits one row. Below that, groups/actions
// wrap to separate rows without overlapping; at tiny heights, controls are
// hidden rather than drawn over neighbouring modules.
inline LoopToolbarLayout makeLoopToolbarLayout (int width, int height) noexcept
{
    LoopToolbarLayout out;
    constexpr int pad = 8, rowH = 27, rowGap = 6;
    constexpr int labelW = 54, stepW = 26, frameW = 80, gap = 4;
    constexpr int groupW = labelW + stepW * 2 + frameW + gap * 3; // 198
    constexpr int actionW = 100 + 6 + 78 + 6 + 78; // 268
    if (width < groupW + pad * 2 || height < rowH + pad * 2)
        return out;

    auto placeGroup = [&] (int x, int y, bool begin)
    {
        Rect* label = begin ? &out.beginLabel : &out.endLabel;
        Rect* minus = begin ? &out.beginMinus : &out.endMinus;
        Rect* field = begin ? &out.beginFrame : &out.endFrame;
        Rect* plus = begin ? &out.beginPlus : &out.endPlus;
        *label = {x, y, labelW, rowH};
        x += labelW + gap;
        *minus = {x, y, stepW, rowH};
        x += stepW + gap;
        *field = {x, y, frameW, rowH};
        x += frameW + gap;
        *plus = {x, y, stepW, rowH};
    };
    auto placeActions = [&] (int x, int y)
    {
        out.preview = {x, y, 100, rowH};
        out.apply = {x + 106, y, 78, rowH};
        out.cancel = {x + 190, y, 78, rowH};
    };
    const int available = width - 2 * pad;
    const int top = pad;

    if (available >= groupW * 2 + 12 + actionW + 14)
    {
        out.requiredHeight = rowH + 2 * pad;
        if (height < out.requiredHeight) return {};
        placeGroup (pad, top, true);
        placeGroup (pad + groupW + 12, top, false);
        placeActions (pad + groupW * 2 + 26, top);
    }
    else if (available >= groupW * 2 + 12)
    {
        out.requiredHeight = rowH * 2 + rowGap + 2 * pad;
        if (height < out.requiredHeight) return {};
        placeGroup (pad, top, true);
        placeGroup (pad + groupW + 12, top, false);
        if (available >= actionW)
            placeActions (pad, top + rowH + rowGap);
        else
            out.requiredHeight = 0; // No safe complete layout at this width.
    }
    else if (available >= actionW)
    {
        out.requiredHeight = rowH * 3 + rowGap * 2 + 2 * pad;
        if (height < out.requiredHeight) return {};
        placeGroup (pad, top, true);
        placeGroup (pad, top + rowH + rowGap, false);
        placeActions (pad, top + (rowH + rowGap) * 2);
    }
    else
    {
        // Narrow view: three vertically stacked action buttons; never crop.
        out.requiredHeight = rowH * 5 + rowGap * 4 + 2 * pad;
        if (height < out.requiredHeight) return {};
        placeGroup (pad, top, true);
        placeGroup (pad, top + rowH + rowGap, false);
        const auto y = top + (rowH + rowGap) * 2;
        out.preview = {pad, y, 100, rowH};
        out.apply = {pad, y + rowH + rowGap, 78, rowH};
        out.cancel = {pad, y + (rowH + rowGap) * 2, 78, rowH};
    }

    out.fits = out.noOverlap (width, height)
               && out.beginFrame.visible() && out.endFrame.visible()
               && out.preview.visible() && out.apply.visible() && out.cancel.visible();
    return out;
}
} // namespace liberty::loop_v015
