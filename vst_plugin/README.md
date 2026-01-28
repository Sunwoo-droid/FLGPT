# FLGPT VST Plugin

AI-powered beat creation plugin for FL Studio using natural language.

## Overview

FLGPT is a VST3 plugin that allows users to create and edit beats using natural language prompts. It integrates with LLM APIs (OpenAI GPT and Anthropic Claude) to translate musical descriptions into MIDI patterns that play through FL Studio instruments.

## Features

- **Natural Language Beat Creation**: Describe beats in plain English
- **Iterative Editing**: Adjust individual components (kick, snare, hi-hat, etc.) without affecting others
- **Multiple LLM Support**: Works with both OpenAI and Anthropic APIs
- **Real-time MIDI Output**: Generates MIDI directly to FL Studio channels
- **Session State Management**: Remembers what you've created for easy editing

## Building

### Prerequisites

- JUCE Framework (v7.0 or later)
- CMake (v3.22 or later)
- C++17 compatible compiler
- FL Studio 2025 (for testing)

### Build Steps

1. Clone or download JUCE framework
2. Place JUCE in the `vst_plugin` directory (or update CMakeLists.txt path)
3. Create build directory:
   ```bash
   mkdir build
   cd build
   ```
4. Configure with CMake:
   ```bash
   cmake ..
   ```
5. Build:
   ```bash
   cmake --build .
   ```

### macOS

The plugin will be built as a `.vst3` bundle. Copy it to:
```
~/Library/Audio/Plug-Ins/VST3/
```

### Windows

The plugin will be built as a `.vst3` folder. Copy it to:
```
C:\Program Files\Common Files\VST3\
```

## Usage

1. **Load the Plugin**: Add FLGPT to a channel in FL Studio
2. **Add an Instrument**: Load FLEX, Serum, or any VST instrument after FLGPT
3. **Configure API**: Click "Settings" to enter your OpenAI or Anthropic API key
4. **Create Beats**: Type prompts like "Create a chill lo-fi beat" or "Make a trap beat at 140 BPM"
5. **Edit Components**: Click on instrument names to adjust them individually

## API Configuration

The plugin supports both OpenAI and Anthropic APIs:

- **OpenAI**: Requires an API key from https://platform.openai.com
- **Anthropic**: Requires an API key from https://console.anthropic.com

Set your API key and preferred provider in the plugin settings.

## Architecture

- **PluginProcessor**: Handles audio/MIDI processing and plugin state
- **PluginEditor**: Main UI window with embedded chat interface
- **ChatComponent**: Chat UI for user interaction
- **LLMService**: Base class for LLM API integration
- **OpenAIService / AnthropicService**: Specific API implementations
- **SessionState**: Manages current beat structure and components
- **MIDIGenerator**: Converts beat patterns to MIDI events

## Development Notes

- MIDI is generated in real-time during playback
- Each instrument component uses a separate MIDI channel
- Drums use MIDI channel 10 (GM standard)
- Pattern arrays represent 16th note subdivisions
- Session state is saved with FL Studio projects

## License

[Your License Here]

