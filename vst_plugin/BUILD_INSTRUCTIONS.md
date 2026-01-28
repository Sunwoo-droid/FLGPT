# FLGPT VST Plugin - Build Instructions

## Quick Start

### Option 1: Using JUCE Projucer (Recommended for beginners)

1. Download and install [JUCE](https://juce.com/get-juce)
2. Open JUCE Projucer
3. Create a new project or open the `.jucer` file (if provided)
4. Configure project settings:
   - Plugin format: VST3
   - Company name: FLGPT
   - Plugin name: FLGPT
5. Add all source files from `Source/` directory
6. Save and open in your IDE (Xcode, Visual Studio, etc.)
7. Build the project

### Option 2: Using CMake

1. Ensure JUCE is available (either as submodule or downloaded)
2. Update `CMakeLists.txt` if JUCE path differs
3. Create build directory:
   ```bash
   cd vst_plugin
   mkdir build
   cd build
   ```
4. Configure:
   ```bash
   cmake ..
   ```
5. Build:
   ```bash
   cmake --build .
   ```

## Required JUCE Modules

The plugin uses these JUCE modules:
- juce_audio_basics
- juce_audio_devices
- juce_audio_formats
- juce_audio_plugin_client
- juce_audio_processors
- juce_audio_utils
- juce_core
- juce_data_structures
- juce_events
- juce_graphics
- juce_gui_basics
- juce_gui_extra

## Platform-Specific Notes

### macOS
- Minimum macOS version: 10.13
- Build with Xcode or CMake
- Install location: `~/Library/Audio/Plug-Ins/VST3/`

### Windows
- Visual Studio 2019 or later recommended
- Install location: `C:\Program Files\Common Files\VST3\`

### Linux
- GCC 7+ or Clang 6+ required
- Install location: `~/.vst3/` or `/usr/lib/vst3/`

## Testing

1. Build the plugin
2. Copy `.vst3` bundle/folder to appropriate location
3. Open FL Studio 2025
4. Add FLGPT to a channel
5. Configure API key in plugin settings
6. Test beat generation

## Troubleshooting

### Plugin doesn't appear in FL Studio
- Ensure plugin is in correct VST3 directory
- Check FL Studio's plugin scan settings
- Verify plugin was built for correct architecture (64-bit)

### API calls fail
- Verify API key is set correctly
- Check internet connection
- Review error messages in plugin UI

### MIDI not playing
- Ensure an instrument is loaded after FLGPT in the channel
- Check MIDI channel routing
- Verify pattern data was generated correctly

