#!/usr/bin/env python3
"""Verify native JUCE stress metrics; do not pretend DAC/slow-disk QA passed."""
import argparse
import json
import math
from pathlib import Path

FIELDS = ('process_cpu_pct', 'callback_budget_avg_pct', 'callback_peak_ms',
          'callback_over_budget', 'peak_rss_mb', 'underflow_blocks',
          'decoder_errors', 'urgent_dropped', 'dropped_pages',
          'nonfinite_samples', 'suspicious_silent_sustain_blocks')
def evaluate(paths):
    result = {}
    for path in paths:
        data = json.loads(Path(path).read_text(encoding='utf-8'))
        scene = data['scenario']
        if scene not in ('poly64', 'switch', 'slice') or scene in result:
            raise ValueError('unknown or repeated scenario: ' + scene)
        if data['zones_at_start'] != 64 or data['blocks'] <= 0:
            raise ValueError(scene + ': native renderer did not render 64-zone bank')
        for field in FIELDS:
            value = data[field]
            if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
                raise ValueError(f'{scene}: invalid {field}')
        if data['nonfinite_samples']:
            raise ValueError(f'{scene}: nonfinite audio')
        incident = any(data[k] > 0 for k in ('callback_over_budget',
                      'underflow_blocks','decoder_errors','urgent_dropped','dropped_pages'))
        if scene == 'poly64':
            incident = incident or data['suspicious_silent_sustain_blocks'] > 0
        data['streaming_qualified'] = not incident
        result[scene] = data
    if set(result) != {'poly64','switch','slice'}:
        raise ValueError('Missing one or more native test scenarios')
    return {'engine': 'Native JUCE processBlock, real-time paced, no audio hardware',
            'qualified_on_ci_machine': all(x['streaming_qualified'] for x in result.values()),
            'slow_disk_and_DAW_qualified': False,
            'limits': 'Process CPU including workers; callback wall time, peak RSS and engine underflows. No actual DAC or USB disk tested.',
            'scenarios': result}

if __name__ == '__main__':
    p = argparse.ArgumentParser()
    p.add_argument('reports', nargs='+', type=Path)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    report = evaluate(a.reports)
    a.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    for name, item in report['scenarios'].items():
        print(f'{name}: CPU={item["process_cpu_pct"]:.1f}% peakRSS={item["peak_rss_mb"]:.1f}MB '
              f'overbudget={item["callback_over_budget"]} underflow={item["underflow_blocks"]} '
              f'qualified={item["streaming_qualified"]}')
    print('Mac CI qualification:', report['qualified_on_ci_machine'])
    print('Slow disk and DAW: NOT TESTED')
