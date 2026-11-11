#!/usr/bin/env python3
"""Liberty ROMpler v0.37 - non-destructive smart multisample bank import.

Example:
 python3 tools/smart_import_v037.py --bank-folder Banks/MyBank \
   --bank-id mybank --bank-name "My Samples" --write

Default: DRY RUN, no files changed. Every mapping and ambiguous name is shown
before --write. Only bank manifest JSON is written; WAV data is never modified.
"""
import argparse
import json
import os
import re
import sys
import wave
from collections import defaultdict
from pathlib import Path

ID = re.compile(r"^[A-Za-z0-9_-]{1,80}$")
NOTE = re.compile(r"(?<![A-Za-z0-9])([A-G])([#b]?)(-?\d)(?!\d)", re.I)
MIDI = re.compile(r"(?:^|[_\-\s])(?:MIDI|NOTE|N)(\d{1,3})(?=$|[_\-\s])", re.I)
VEL = re.compile(r"(?:^|[_\-\s])(?:VEL|V)(\d{1,3})[-_](\d{1,3})(?=$|[_\-\s])", re.I)
RR = re.compile(r"(?:^|[_\-\s])(?:RR|TAKE)(\d{1,2})(?=$|[_\-\s])", re.I)
ART = re.compile(r"(?:^|[_\-\s])ART(SUSTAIN|STACCATO|LEGATO|PIZZICATO|MUTE|TREMOLO)(?=$|[_\-\s])", re.I)
KS = re.compile(r"(?:^|[_\-\s])KS(\d{1,3})(?=$|[_\-\s])", re.I)
ARTICULATIONS = {"SUSTAIN": 1, "STACCATO": 2, "LEGATO": 3,
                 "PIZZICATO": 4, "MUTE": 5, "TREMOLO": 6}
NOTES = {"C": 0, "D": 2, "E": 4, "F": 5,
         "G": 7, "A": 9, "B": 11}


class ImportErrorWithContext(ValueError):
    pass


def get_root(stem):
    named = list(NOTE.finditer(stem))
    numeric = list(MIDI.finditer(stem))
    if len(named) + len(numeric) != 1:
        raise ImportErrorWithContext(
            "ambiguous/missing root note: use _C4_, _F#3_, or _N060_")
    if numeric:
        token = numeric[0]
        note = int(token.group(1))
    else:
        token = named[0]
        octave = int(token.group(3))
        semi = NOTES[token.group(1).upper()] + (
            1 if token.group(2) == "#" else -1 if token.group(2) == "b" else 0)
        note = 12 * (octave + 1) + semi  # C4 == MIDI 60
    if not 0 <= note <= 127:
        raise ImportErrorWithContext("MIDI root note outside 0..127")
    prefix = stem[:token.start()].strip(" _-").lower()
    prefix = re.sub(r"[^a-z0-9_-]", "-", prefix).strip("_-")
    return note, prefix or "instrument"


def read_meta(path):
    try:
        with wave.open(str(path), "rb") as reader:
            channels, bits, rate, frames = (
                reader.getnchannels(), reader.getsampwidth(), 
                reader.getframerate(), reader.getnframes())
    except (OSError, EOFError, wave.Error) as exc:
        raise ImportErrorWithContext(f"not a valid PCM WAV: {exc}") from exc
    if channels not in (1, 2) or bits != 2 or rate < 22050:
        raise ImportErrorWithContext(
            "expected 16-bit PCM WAV, mono/stereo, at least 22050 Hz")
    if frames < 2:
        raise ImportErrorWithContext("empty WAV")
    # The v0.36 engine retains its 20-second per-zone memory cap.
    # Never silently truncate long sounds and call it disk streaming.
    if frames > rate * 20:
        raise ImportErrorWithContext(
            "WAV longer than 20 s: dedicated disk streaming not active yet")
    return rate, frames


def parse_sample(path, root):
    if path.is_symlink():
        raise ImportErrorWithContext("symlinked WAV excluded for safe bank paths")
    relative = path.relative_to(root).as_posix()
    if "\\" in relative or ".." in path.relative_to(root).parts:
        raise ImportErrorWithContext("unsafe relative path")
    note, instrument = get_root(path.stem)
    matches = list(VEL.finditer(path.stem))
    if len(matches) > 1:
        raise ImportErrorWithContext("multiple velocity ranges")
    low, high = (int(matches[0].group(1)), int(matches[0].group(2))) if matches else (1, 127)
    if not 1 <= low <= high <= 127:
        raise ImportErrorWithContext("invalid velocity range (use V001-063)")
    takes = list(RR.finditer(path.stem))
    if len(takes) > 1:
        raise ImportErrorWithContext("multiple Round Robin indices")
    rr = int(takes[0].group(1)) if takes else 0
    if rr > 16 or (takes and rr < 1):
        raise ImportErrorWithContext("RR must be in 01..16")
    articulations = list(ART.finditer(path.stem))
    keyswitches = list(KS.finditer(path.stem))
    if len(articulations) > 1 or len(keyswitches) > 1:
        raise ImportErrorWithContext("multiple articulation/keyswitch tokens")
    art = ARTICULATIONS[articulations[0].group(1).upper()] if articulations else 0
    if keyswitches and not art:
        raise ImportErrorWithContext("KS requires a matching ART token")
    ks = int(keyswitches[0].group(1)) if keyswitches else (23 + art if art else -1)
    if ks > 127:
        raise ImportErrorWithContext("keyswitch note outside MIDI 0..127")
    rate, frames = read_meta(path)
    zone = {
        "file": relative, "root_note": note, "low_note": 0, "high_note": 127,
        "low_velocity": low, "high_velocity": high,
        "loop_start": -1, "loop_end": -1, "gain": 1.0,
    }
    if rr:
        zone["rr_group"], zone["rr_index"] = 1, rr
    if art:
        zone["articulation"], zone["keyswitch_note"] = art, ks
    return instrument, zone, (rate, frames)


def create_manifest(folder, bank_id, bank_name):
    if not folder.is_dir():
        raise ImportErrorWithContext("bank folder does not exist")
    if not ID.fullmatch(bank_id):
        raise ImportErrorWithContext("bank ID must be 1..80 ASCII letters, digits, - or _")
    if not bank_name.strip():
        raise ImportErrorWithContext("bank name cannot be empty")
    files = sorted(folder.rglob("*"), key=lambda x: x.as_posix().lower())
    files = [x for x in files if x.suffix.lower() == ".wav"]
    if not files or len(files) > 8192:
        raise ImportErrorWithContext("expected 1..8192 WAV files")
    groups = defaultdict(list)
    errors = []
    for path in files:
        try:
            name, zone, meta = parse_sample(path, folder)
            groups[name].append(zone)
        except ImportErrorWithContext as exc:
            errors.append(f"{path.relative_to(folder)}: {exc}")
    if errors:
        raise ImportErrorWithContext("\n".join(errors[:40]) + (
            f"\n... {len(errors) - 40} additional errors" if len(errors) > 40 else ""))
    presets = []
    for instrument, zones in sorted(groups.items()):
        # Same note boundaries across velocities, RR and articulations.
        roots = sorted({zone["root_note"] for zone in zones})
        ranges = {}
        for i, note in enumerate(roots):
            lo = 0 if i == 0 else (roots[i - 1] + note) // 2 + 1
            hi = 127 if i == len(roots) - 1 else (note + roots[i + 1]) // 2
            ranges[note] = lo, hi
        seen = defaultdict(set)
        for zone in zones:
            zone["low_note"], zone["high_note"] = ranges[zone["root_note"]]
            key = (zone["root_note"], zone["low_velocity"],
                   zone["high_velocity"], zone.get("articulation", 0))
            slot = zone.get("rr_index", 0)
            if slot in seen[key]:
                raise ImportErrorWithContext(
                    f"{instrument}: duplicate RR take {slot} for MIDI {key[0]}")
            seen[key].add(slot)
        for key, takes in seen.items():
            if 0 in takes and len(takes) > 1:
                raise ImportErrorWithContext(
                    f"{instrument}: mixed RR/non-RR takes at MIDI {key[0]}")
        zones.sort(key=lambda z: (z["root_note"], z["low_velocity"],
                                  z.get("articulation", 0), z.get("rr_index", 0)))
        # Current bank format limits each preset to 32 zones.
        for chunk, start in enumerate(range(0, len(zones), 32)):
            token = re.sub(r"[^a-z0-9_-]", "-", instrument.lower()).strip("-")[:60]
            preset_id = token + (f"-part{chunk + 1}" if chunk else "")
            presets.append({
                "id": preset_id,
                "name": instrument.replace("-", " ").replace("_", " ").title() +
                        (f" (Part {chunk + 1})" if chunk else ""),
                "category": "Imported Multisample",
                "zones": zones[start:start + 32]
            })
    if len(presets) > 256:
        raise ImportErrorWithContext("too many preset chunks (limit 256)")
    return {"format": "liberty-rompler-bank-v1", "id": bank_id,
            "name": bank_name.strip(), "version": "1.0.0",
            "presets": presets}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bank-folder", type=Path, required=True)
    parser.add_argument("--bank-id", required=True)
    parser.add_argument("--bank-name", required=True)
    parser.add_argument("--write", action="store_true",
                        help="write manifest.json (DRY RUN otherwise)")
    parser.add_argument("--replace", action="store_true",
                        help="replace existing manifest (requires --write)")
    options = parser.parse_args(argv)
    folder = options.bank_folder.expanduser().resolve()
    try:
        manifest = create_manifest(folder, options.bank_id, options.bank_name)
        dest = folder / "manifest.json"
        print(f"Smart Import: {sum(len(x['zones']) for x in manifest['presets'])} WAV zones, "
              f"{len(manifest['presets'])} presets")
        for item in manifest["presets"]:
            print(f"  {item['name']}: {len(item['zones'])} zones")
        if not options.write:
            print("DRY RUN. No file changed. Add --write after reviewing the mapping.")
            return 0
        if dest.exists() and not options.replace:
            raise ImportErrorWithContext("manifest.json exists; use --replace explicitly")
        pending = folder / "manifest.json.liberty-pending"
        if pending.exists():
            raise ImportErrorWithContext("pending manifest exists; remove it manually")
        with pending.open("x", encoding="utf-8") as handle:
            json.dump(manifest, handle, ensure_ascii=False, indent=2)
            handle.write("\n")
        os.replace(pending, dest)
        print(f"Manifest written: {dest}")
        return 0
    except (ImportErrorWithContext, OSError) as exc:
        print(f"IMPORT FAILED: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
