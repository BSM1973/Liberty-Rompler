#!/usr/bin/env python3
"""Liberty v0.42: prioritize demanded streaming pages and expose health stats.
Apply once after v0.41 pool integration. No new UI, preset or bank fields.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source"


def edit(path, modifications):
    target = SRC / path
    source = target.read_text(encoding="utf-8")
    for before, after in modifications:
        matches = source.count(before)
        if matches != 1:
            raise SystemExit(
                f"v0.42 {path}: expected 1 exact anchor but found {matches}: "
                f"{before[:100]!r}")
        source = source.replace(before, after, 1)
    target.write_text(source, encoding="utf-8")


edit("StreamingPageCacheV039.h", [
    (
        "    // Real-time safe MPSC request mailbox.",
        """    // Approximate residency check: atomic metadata only, no buffer access.
    // A concurrent eviction can invalidate the result immediately, which is
    // acceptable for scheduling, never for reading sample data.
    bool isPageResident(std::int64_t pageIndex) const noexcept {
        if (pageIndex < 0) return false;
        for (std::size_t i = 0; i < PageSlots; ++i) {
            const auto& slot = pages[i];
            if (slot.state.load(std::memory_order_seq_cst) == 2 &&
                slot.page.load(std::memory_order_seq_cst) == pageIndex)
                return true;
        }
        return false;
    }

    // Real-time safe MPSC request mailbox.""",
    ),
])

edit("StreamedZoneV040.h", [
    (
        '#include "StreamingPoolV041.h"',
        '#include "StreamingPoolV041.h"\n#include "StreamingHealthV042.h"',
    ),
    (
        "    explicit StreamedZone(std::unique_ptr<juce::AudioFormatReader> source)\n"
        "        : reader(std::move(source), static_cast<int>(pageFrames)) {}",
        """    explicit StreamedZone(std::unique_ptr<juce::AudioFormatReader> source)
        : reader(std::move(source), static_cast<int>(pageFrames)) {
        for (auto& pending : urgentPages)
            pending.store(-1, std::memory_order_relaxed);
    }""",
    ),
    (
        "    // Audio callback operations: bounded cache reads/atomic page requests.",
        """    // Highest-priority read requests for the frames voices are consuming
    // NOW, including nonzero SLICE starts. Bounded MPSC mailbox, no locking.
    // Different simultaneous voice positions do not overwrite one another.
    void requestCritical(std::int64_t frame) noexcept {
        if (frame < 0 || frame >= totalFrames()) return;
        const auto page = frame / static_cast<std::int64_t>(pageFrames);
        if (cache.isPageResident(page)) return;
        for (const auto& item : urgentPages)
            if (item.load(std::memory_order_relaxed) == page) return;
        const auto first = urgentCursor.fetch_add(1, std::memory_order_relaxed);
        for (std::size_t i = 0; i < urgentCapacity; ++i) {
            auto& item = urgentPages[(first + i) % urgentCapacity];
            std::int64_t unused = -1;
            if (item.compare_exchange_strong(unused, page,
                std::memory_order_release, std::memory_order_relaxed)) {
                urgentRequests.fetch_add(1, std::memory_order_relaxed);
                return;
            }
        }
        urgentDrops.fetch_add(1, std::memory_order_relaxed);
        (void) cache.requestPage(page); // fallback also stays nonblocking
    }

    // Audio callback operations: bounded cache reads/atomic page requests.""",
    ),
    (
        "    Cache::Stats stats() const noexcept { return cache.stats(); }",
        """    Cache::Stats stats() const noexcept { return cache.stats(); }
    // A snapshot for non-real-time diagnostics (future UI and load tests).
    liberty::stream_v042::HealthSnapshot health() const noexcept {
        const auto s = cache.stats();
        return {s.hit, s.miss, s.request, s.droppedRequest, s.published,
                readErrors.load(std::memory_order_relaxed),
                urgentRequests.load(std::memory_order_relaxed),
                urgentReads.load(std::memory_order_relaxed),
                urgentDrops.load(std::memory_order_relaxed),
                underflowBlocks.load(std::memory_order_relaxed)};
    }
    void markUnderflowBlock() noexcept {
        underflowBlocks.fetch_add(1, std::memory_order_relaxed);
    }""",
    ),
    (
        "    bool serviceOneRequest() noexcept override {",
        """    bool serviceUrgentRequest() noexcept override {
        for (auto& pending : urgentPages) {
            const auto page = pending.exchange(-1, std::memory_order_acq_rel);
            if (page < 0) continue;
            if (cache.isPageResident(page)) return true;
            if (loadPage(page)) {
                urgentReads.fetch_add(1, std::memory_order_relaxed);
                const auto last = (totalFrames() - 1)
                    / static_cast<std::int64_t>(pageFrames);
                if (page < last) (void)loadPage(page + 1);
            }
            return true; // work done even if the disk reports a read error
        }
        return false;
    }

    bool serviceOneRequest() noexcept override {""",
    ),
    (
        "        if (!cache.pollRequest(page)) return false;\n"
        "        if (loadPage(page)) {",
        "        if (!cache.pollRequest(page)) return false;\n"
        "        if (!cache.isPageResident(page) && loadPage(page)) {",
    ),
    (
        "    bool loadPage(std::int64_t page) noexcept {\n",
        "    bool loadPage(std::int64_t page) noexcept {\n"
        "        if (cache.isPageResident(page)) return true;\n",
    ),
    (
        "    Reader reader;   // one decoder, owned by this zone, worker-only access",
        """    static constexpr std::size_t urgentCapacity = 16;
    std::array<std::atomic<std::int64_t>, urgentCapacity> urgentPages;
    std::atomic<std::size_t> urgentCursor {0};
    std::atomic<std::uint64_t> urgentRequests {0}, urgentReads {0},
                               urgentDrops {0}, underflowBlocks {0};

    Reader reader;   // one decoder, owned by this zone, worker-only access""",
    ),
])

edit("SampleEngine.h", [
    ("#include <cstdint>", "#include <cstdint>\n#include <cmath>"),
    (
        "    void requestStreamAhead(double pos) const noexcept {",
        """    // Front-of-queue demand, available even during nonzero SLICE seeks.
    void requestStreamCritical(double pos) const noexcept {
        if (streamed && std::isfinite(pos) && pos >= 0.0 &&
            pos < (double)totalFrames)
            streamed->requestCritical((std::int64_t)pos);
    }
    void reportStreamUnderflowBlock() const noexcept {
        if (streamed) streamed->markUnderflowBlock();
    }
    liberty::stream_v042::HealthSnapshot streamingHealth() const noexcept {
        return streamed ? streamed->health()
                        : liberty::stream_v042::HealthSnapshot{};
    }
    void requestStreamAhead(double pos) const noexcept {""",
    ),
])

edit("SampleEngine.cpp", [
    (
        "    if (s->isStreamed()) {\n"
        "        s->requestStreamAhead(position);",
        "    if (s->isStreamed()) {\n"
        "        s->requestStreamCritical(position);\n"
        "        s->requestStreamAhead(position);",
    ),
    (
        "    s->requestStreamAhead(position);\n"
        "    s->requestStreamAhead(position+count*timeRatio+grainFrames*ratio);",
        "    s->requestStreamCritical(position);\n"
        "    s->requestStreamAhead(position);\n"
        "    s->requestStreamAhead(position+count*timeRatio+grainFrames*ratio);",
    ),
    (
        "    if (s->getLoopStart()>=0) s->requestStreamAhead(s->getLoopStart());",
        """    if (s->getLoopStart()>=0) {
        s->requestStreamAhead(s->getLoopStart());
        // Time-critical crossfade/loop-head demand near a wrap boundary.
        if (position + count*timeRatio >=
            s->getLoopEnd() - 2.0*s->getLoopCrossfadeFrames())
            s->requestStreamCritical(s->getLoopStart());
    }
    bool underflowedThisBlock = false;""",
    ),
    (
        "        } else {\n"
        "            streamGain=juce::jmax(0.0f,streamGain-1.0f/64.0f);\n"
        "            left=lastStreamLeft; right=lastStreamRight;",
        "        } else {\n"
        "            if (!underflowedThisBlock) {\n"
        "                s->reportStreamUnderflowBlock();\n"
        "                underflowedThisBlock = true;\n"
        "            }\n"
        "            streamGain=juce::jmax(0.0f,streamGain-1.0f/64.0f);\n"
        "            left=lastStreamLeft; right=lastStreamRight;",
    ),
])

print("Liberty v0.42: critical page scheduling and streaming health integrated.")
