#!/usr/bin/env python3
"""Diagnostic-only synthetic-runner reference versus actual 64-voice stress.

A reference overrun supports a runner/scheduler interruption hypothesis but
cannot establish its cause. Zero reference overruns cannot rule it out.
Never changes or overrides the real JUCE strict qualification.
"""
import argparse
import json
import math
from pathlib import Path


def validate(report, scenario):
    if report.get('scenario') != scenario:
        raise ValueError(f'expected {scenario} report')
    for field in ('rate_hz', 'block_frames', 'elapsed_wall_sec'):
        value = report.get(field)
        if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value <= 0:
            raise ValueError(f'invalid {field}: {value}')
    if report['elapsed_wall_sec'] < 55.:
        raise ValueError(f'{scenario}: reference shorter than 55 seconds')
    overruns = report.get('callback_over_budget')
    if isinstance(overruns, bool) or not isinstance(overruns, int) or overruns < 0:
        raise ValueError('invalid callback_over_budget')
    spikes = report.get('callback_spikes')
    if not isinstance(spikes, list):
        raise ValueError('missing callback spikes')
    if report.get('spikes_truncated', False):
        raise ValueError('truncated spike trace cannot be compared')
    if len(spikes) != overruns:
        raise ValueError('spike list does not match overrun counter')
    budget = 1000 * report['block_frames'] / report['rate_hz']
    for spike in spikes:
        wall = spike.get('callback_ms')
        cpu = spike.get('thread_cpu_ms')
        if not all(isinstance(x, (int, float)) and not isinstance(x, bool) and math.isfinite(x) for x in (wall, cpu)):
            raise ValueError('invalid spike timing')
        if wall <= budget or cpu < 0 or cpu > wall + .05:
            raise ValueError('inconsistent spike timing')
    return budget


def compare(reference, actual):
    rb = validate(reference, 'scheduler_reference')
    ab = validate(actual, 'poly64')
    if abs(rb-ab) > .001:
        raise ValueError('reference and JUCE callback budgets differ')
    if reference.get('qos_interactive_confirmed') is not True:
        raise ValueError('reference macOS QoS was not confirmed')
    if not 0 < reference.get('synthetic_cpu_work_target_ms', 0) < rb:
        raise ValueError('invalid synthetic work amount')
    return {
        'reference_overruns': reference['callback_over_budget'],
        'real_64voice_overruns': actual['callback_over_budget'],
        'reference_spike_observed': reference['callback_over_budget'] > 0,
        'real_stress_passed': actual['callback_over_budget'] == 0,
        'evidence_of_external_runner_jitter': reference['callback_over_budget'] > 0,
        'root_cause_proven': False,
        'qualification_override': False,
        'note': 'Different loads and timings. Reference spikes support, but cannot prove, scheduler jitter; zero reference spikes do not exclude it. The actual JUCE strict gate is unchanged.'
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('reference', type=Path)
    parser.add_argument('real_poly64', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    data = compare(json.loads(args.reference.read_text('utf-8')),
                   json.loads(args.real_poly64.read_text('utf-8')))
    args.output.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(data, indent=2))

if __name__ == '__main__':
    main()
