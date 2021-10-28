cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

project(JuceUnityPluginTest VERSION 0.0.1)

add_subdirectory(Dependencies/JUCE)                    # If you've put JUCE in a subdirectory called JUCE

set(TARGET_NAME MyJuceUnityPlugin)

juce_add_plugin(${TARGET_NAME}
    VERSION 0.0.1                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    # COMPANY_NAME ...                          # Specify the name of the plugin's author
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    MICROPHONE_PERMISSION_ENABLED TRUE
    PLUGIN_MANUFACTURER_CODE Juce               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Dem0                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    FORMATS AU VST3 Standalone Unity            # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    PRODUCT_NAME ${TARGET_NAME})        # The name of the final executable, which can differ from the target name

set(BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

list(APPEND SOURCE_LIST
    ${BASE_DIR}/PluginEditor.cpp
    ${BASE_DIR}/PluginEditor.h
    ${BASE_DIR}/PluginProcessor.cpp
    ${BASE_DIR}/PluginProcessor.h
    )

source_group(TREE ${BASE_DIR} FILES ${SOURCE_LIST})

target_sources(${TARGET_NAME}
    PRIVATE
    ${SOURCE_LIST}
    )

target_compile_definitions(${TARGET_NAME}
    PUBLIC
    # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
    JUCE_VST3_CAN_REPLACE_VST2=0)

juce_add_binary_data(AudioPluginData SOURCES
    assets/C.wav
    assets/D.wav
    assets/E.wav
    assets/F.wav
    assets/G.wav
    assets/A.wav
    assets/B.wav
    )

target_link_libraries(${TARGET_NAME}
    PRIVATE
    AudioPluginData           # If we'd created a binary data target, we'd link to it here
    juce::juce_audio_utils
    juce::juce_audio_basics
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
