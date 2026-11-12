#!/usr/bin/env python3
"""Opt-in native stress app, AFTER v0.43 source overlays, BEFORE CMake."""
from pathlib import Path
p = Path(__file__).resolve().parent.parent / 'CMakeLists.txt'
s = p.read_text(encoding='utf-8')
if 'LIBERTY_BUILD_STRESS_V044' in s:
    raise SystemExit('v0.44 stress target already integrated')
s += '''
# macOS engine stress runner, isolated from AU/VST3/Standalone deliverables.
option(LIBERTY_BUILD_STRESS_V044 "Build offline engine stress runner" OFF)
if(LIBERTY_BUILD_STRESS_V044)
    juce_add_console_app(LibertyStressV044 PRODUCT_NAME "Liberty Stress v0.44")
    juce_generate_juce_header(LibertyStressV044)
    target_sources(LibertyStressV044 PRIVATE tests/rompler_audio_stress_v044.cpp)
    set_target_properties(LibertyStressV044 PROPERTIES OUTPUT_NAME "liberty-stress-v044")
    target_include_directories(LibertyStressV044 PRIVATE Source)
    target_compile_definitions(LibertyStressV044 PRIVATE
        JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
    target_link_libraries(LibertyStressV044 PRIVATE
        LibertyRompler
        juce::juce_audio_utils
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
endif()
'''
p.write_text(s, encoding='utf-8')
print('v0.44 optional native stress app integrated.')
