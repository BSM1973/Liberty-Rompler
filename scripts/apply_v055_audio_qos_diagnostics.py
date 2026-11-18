#!/usr/bin/env python3
"""Run the offline JUCE audio-callback thread at explicit macOS interactive QoS.

The GitHub runner does not provide a CoreAudio host or real-time workgroup.
This changes only the OFFLINE TEST HARNESS, never plugin DSP or release gates.
Do not mistake interactive QoS for a real-time hardware callback.
"""
from pathlib import Path
p=Path("tests/rompler_audio_stress_v044.cpp")
s=p.read_text("utf-8")
def once(before,after):
    global s
    if s.count(before)!=1:
        raise SystemExit(f"macOS offline QoS anchor mismatch: {before[:90]}")
    s=s.replace(before,after,1)
once("#include <mach/mach.h>", """#include <mach/mach.h>
#include <pthread/qos.h>""")
once("        juce::ScopedJuceInitialiser_GUI juceGui;", """        juce::ScopedJuceInitialiser_GUI juceGui;
#ifdef __APPLE__
        // This very thread calls processBlock. Avoid testing audio rendering
        // at the default utility-like scheduling priority of a CI shell job.
        // Interactive QoS is NOT the CoreAudio time-constraint policy.
        const int audioQosError=pthread_set_qos_class_self_np(
            QOS_CLASS_USER_INTERACTIVE,0);
        insist(audioQosError==0,"Unable to apply macOS interactive audio-test QoS");
        std::cout<<"LIBERTY_NATIVE_AUDIO_QOS: user_interactive, set_ok=true\\n";
#endif""")
p.write_text(s,"utf-8")
print("macOS offline JUCE callback thread: interactive QoS requested and verified.")
