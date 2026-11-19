#!/usr/bin/env python3
"""Generate 100 original multisampled Liberty ROMpler v0.55 Factory Starter presets.

Independent of JUCE. The v1 manifest can be imported by the existing bank browser.
No copyrighted samples, external soundfonts, or preset cloning.
"""
from __future__ import annotations
import argparse
import json
import math
import re
import wave
from pathlib import Path
import numpy as np

BANK_ID='liberty-factory-starter-100-v055'
BANK_DIR='Factory-Starter-v055-100'
SAMPLE_RATE=32000
ROOTS=(36, 52, 68, 84)
RANGES=((0,44),(45,60),(61,76),(77,127))
LIBRARY={
 'Analog Basses': ['Submarine','Copper Pulse','Round Current','Velvet Bass','Circuit Throb','Deep Meridian','Amber Drive','Solid Ground','Pulse Theory','Midnight Circuit'],
 'Electric Keys': ['Glass Rhodes','Warm Tines','Amber Keys','Soft Hammer','Electric Velvet','Moonlight Piano','City Nights','Crystal Stage','Tape Keys','Silver Chimes'],
 'Cinematic Pads': ['Orion Veil','Blue Aurora','Nebula Drift','Polar Horizon','Lunar Bloom','Cloud Cathedral','Wide Cosmos','Silent Planet','Harmonic Sea','Stellar Choir'],
 'Modern Leads': ['Solar Arc','Vector Voice','Neon Blade','Analog Spear','Hyper Focus','Electric Thread','Sonic Arrow','Ion Driver','Golden Signal','Lumen Lead'],
 'Plucked Textures': ['Velvet Pluck','Digital Harp','Copper Droplet','Prism Pluck','Echo Strings','Soft Kalimba','Glimmer Tap','Arcade Pearl','Bright Vessel','Felt Dust'],
 'Organic Motion': ['Woodland Breath','Rain Lantern','Coastal Air','Distant Reeds','Moving Grain','Wind Sculpture','Copper Leaves','Soft Embers','Fluid Motion','Earth Current'],
 'Bells & Mallets': ['Silver Bell','Hollow Bronze','Glass Marimba','Northern Bells','Crystal Drops','Copper Mallet','Arctic Glock','Resonant Steel','Golden Chime','Temple Glass'],
 'Synth Strings': ['Warm Ensemble','Eternal Bow','Analog Strings','Celestial Violins','Soft Chamber','Vintage Orchestra','Velvet Section','Luminous String','Grand Motion','String Clouds'],
 'Atmospheres': ['Midnight Rain','Cosmic Dust','Dark Waters','Alien Meadow','Solar Wind','Distant Signal','Subliminal Air','Emerald Space','Ghost Lights','Infinite Dawn'],
 'Rhythmic Pulses': ['Electro Motion','Metro Orbit','Future Beat','Pulse Geometry','Night Sequence','Binary Bloom','Tidal Motion','Clockwork Field','Dynamic Current','Zero Gravity'],
}
assert len(LIBRARY)==10 and sum(map(len,LIBRARY.values()))==100

def slug(s:str)->str:
    return re.sub(r'[^a-z0-9]+','-',s.lower()).strip('-')

def synth(category:str,variant:int,root:int)->tuple[np.ndarray,bool,int,int]:
    """One stereo original PCM sample for a curated preset/root. Normalized 16-bit PCM."""
    ix=list(LIBRARY).index(category)
    sustain=category not in {'Bells & Mallets','Plucked Textures'}
    duration=2.3 if sustain else 2.65
    n=int(SAMPLE_RATE*duration)
    t=np.arange(n,dtype=np.float64)/SAMPLE_RATE
    fundamental=440.0*2.0**((root-69)/12)
    brightness=(variant%10)/9.0
    result=np.zeros(n,dtype=np.float64)
    profiles={
        'Analog Basses':(1.36,0.44,0.10),
        'Electric Keys':(1.73,0.15,0.07),
        'Cinematic Pads':(1.28,0.38,0.11),
        'Modern Leads':(0.88,0.70,0.08),
        'Plucked Textures':(1.48,0.30,0.12),
        'Organic Motion':(1.90,0.18,0.17),
        'Bells & Mallets':(1.13,0.60,0.28),
        'Synth Strings':(1.42,0.24,0.13),
        'Atmospheres':(1.09,0.32,0.21),
        'Rhythmic Pulses':(1.24,0.52,0.11),
    }
    roll,even,noise=profiles[category]
    roll+=0.22*(1-brightness)
    phase=2*np.pi*fundamental*t
    maxharm=min(24,int((SAMPLE_RATE/2-120)/fundamental))
    for h in range(1,maxharm+1):
        amp=(1/h**roll)*(1.0 if h%2 else even)
        amp*=max(0.2,1.0-0.020*h*(1.0-brightness))
        if category in ('Modern Leads','Rhythmic Pulses'):
            amp*=1+0.08*np.sin(2*np.pi*(1+(variant%3))*t+0.23*h)
        elif category in ('Cinematic Pads','Atmospheres','Synth Strings'):
            amp*=1+0.07*np.sin(2*np.pi*(0.43+variant*.037)*t+0.37*h)
        result+=amp*np.sin(h*phase+.25*h*variant+0.07*ix*h)
    rng=np.random.default_rng(73400+ix*2000+variant*90+root)
    attack={
        'Analog Basses':.006,'Electric Keys':.008,'Cinematic Pads':.24,
        'Modern Leads':.011,'Plucked Textures':.003,'Organic Motion':.095,
        'Bells & Mallets':.002,'Synth Strings':.13,'Atmospheres':.20,
        'Rhythmic Pulses':.010,
    }[category]*(.85+.30*brightness)
    env=1-np.exp(-t/max(attack,.001))
    if sustain:
        env*=.78+.22*np.exp(-np.maximum(0,t-attack)/(.4+.10*brightness))
        if category=='Rhythmic Pulses':
            env*=.56+.44*np.maximum(0,np.sin(2*np.pi*(2+variant%4)*t+.3*variant))
        elif category=='Organic Motion':
            env*=.86+.14*np.sin(2*np.pi*(.6+variant*.06)*t)**2
    else:
        tail=(.42+.18*brightness) if category=='Plucked Textures' else (.67+.14*brightness)
        env*=np.exp(-t/tail)
        env*=np.minimum(1,np.maximum(0,(duration-t)/.15))
    transient=rng.standard_normal(n)
    transient=np.convolve(transient,np.array([.16,.22,.24,.22,.16]),mode='same')
    result=(result*env)+(transient*noise*np.exp(-t/((.018+.024*brightness) if not sustain else .075)))
    result/=max(1e-9,np.max(np.abs(result)))
    l=result*(.97+.025*np.sin(2*np.pi*(.40+.03*variant)*t))
    r=result*(.95+.035*np.sin(2*np.pi*(.47+.02*variant)*t+.9))
    stereo=np.stack((l,r),axis=1)
    stereo=np.tanh(stereo*1.35)/np.tanh(1.35)
    stereo*=.34
    pcm=np.asarray(np.round(stereo*32767),dtype='<i2')
    loop_start=int(.80*SAMPLE_RATE) if sustain else -1
    loop_end=int(1.80*SAMPLE_RATE) if sustain else -1
    return pcm,sustain,loop_start,loop_end

def generate(root:Path)->Path:
    bank=root/BANK_DIR
    if bank.exists():
        raise FileExistsError(f'Non-destructive generator: {bank} exists')
    samples=bank/'Samples'
    samples.mkdir(parents=True,exist_ok=False)
    presets=[]
    for category,names in LIBRARY.items():
        for variant,name in enumerate(names):
            ident=slug(name)
            zones=[]
            for pitch,(lo,hi) in zip(ROOTS,RANGES):
                pcm,sustained,loop_start,loop_end=synth(category,variant,pitch)
                filename=f'{ident}-{pitch}.wav'
                with wave.open(str(samples/filename),'wb') as wav:
                    wav.setnchannels(2);wav.setsampwidth(2);wav.setframerate(SAMPLE_RATE)
                    wav.writeframes(pcm.tobytes())
                zone={'file':'Samples/'+filename,'root_note':pitch,
                      'low_note':lo,'high_note':hi,'low_velocity':1,
                      'high_velocity':127,'gain':0.78}
                if sustained:
                    zone['loop_start']=loop_start
                    zone['loop_end']=loop_end
                zones.append(zone)
            presets.append({'id':ident,'name':name,'category':category,'zones':zones})
    assert len(presets)==100 and len({p['id'] for p in presets})==100
    manifest={'format':'liberty-rompler-bank-v1',
              'id':BANK_ID,'name':'Liberty Factory Starter — 100 Presets',
              'version':'0.55.0','license_mode':'demo-unprotected',
              'presets':presets}
    path=bank/'manifest.json'
    path.write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    return path

if __name__=='__main__':
    ap=argparse.ArgumentParser()
    ap.add_argument('--output',type=Path,default=Path('Banks'))
    args=ap.parse_args()
    path=generate(args.output)
    print('Created',path,'with 100 presets and 400 stereo samples')
