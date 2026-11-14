#!/usr/bin/env python3
"""Summarize repeated native SLICE captures; honest wall-vs-thread CPU diagnoses."""
import argparse
import json
from pathlib import Path
import statistics

def summarize(paths):
    runs=[]
    for path in paths:
        d=json.loads(Path(path).read_text(encoding='utf8'))
        assert d['scenario']=='slice', f'Not a SLICE benchmark: {path}'
        assert d['rate_hz']>0 and d['block_frames']>0
        assert d['nonfinite_samples']==0 and d['signal_peak']>0
        budget_ms=1000*d['block_frames']/d['rate_hz']
        cpu_spikes=[]
        offcpu_spikes=[]
        for spike in d.get('callback_spikes',[]):
            thread=spike.get('thread_cpu_ms')
            wall=spike.get('callback_ms')
            if thread is None: continue
            if thread>budget_ms: cpu_spikes.append(spike)
            elif wall>budget_ms: offcpu_spikes.append(spike)
        runs.append({
            'file':str(path), 'underflow_blocks':d['underflow_blocks'],
            'cache_misses':d['cache_misses'],
            'callback_over_budget':d['callback_over_budget'],
            'callback_p99_ms':d['callback_p99_ms'],
            'callback_peak_ms':d['callback_peak_ms'],
            'callback_thread_cpu_p99_ms':d.get('callback_thread_cpu_p99_ms'),
            'callback_thread_cpu_peak_ms':d.get('callback_thread_cpu_peak_ms'),
            'cpu_bound_spikes':cpu_spikes, 'off_cpu_spikes':offcpu_spikes,
            'memory_peak_mb':d['peak_rss_mb'], 'budget_ms':budget_ms
        })
    return {
        'samples':len(runs),
        'runs':runs,
        'underflow_total':sum(a['underflow_blocks'] for a in runs),
        'cache_miss_total':sum(a['cache_misses'] for a in runs),
        'callback_over_budget_total':sum(a['callback_over_budget'] for a in runs),
        'cpu_bound_spikes_total':sum(len(a['cpu_bound_spikes']) for a in runs),
        'off_cpu_spikes_total':sum(len(a['off_cpu_spikes']) for a in runs),
        'p99_median_ms':statistics.median(a['callback_p99_ms'] for a in runs),
        'streaming_qualified_in_native_runner':all(
            a['underflow_blocks']==0 and a['cache_misses']==0 for a in runs),
        'qualified_in_real_daw':False,
        'qualified_on_usb_drive':False
    }

if __name__=='__main__':
    ap=argparse.ArgumentParser()
    ap.add_argument('reports',nargs='+',type=Path)
    ap.add_argument('--output',required=True,type=Path)
    args=ap.parse_args()
    result=summarize(args.reports)
    args.output.parent.mkdir(exist_ok=True,parents=True)
    args.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps({k:v for k,v in result.items() if k!='runs'},indent=2))
    if not result['streaming_qualified_in_native_runner']:
        raise SystemExit('SLICE native underflow or cache miss regression')
