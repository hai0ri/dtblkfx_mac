# DtBlkFx (macOS Universal Edition)

**DtBlkFx** is a Fast Fourier Transform (FFT) based VST plug-in for macOS, originally created by Darrell Tam and updated for modern macOS (Universal Binary) and JUCE by haiori.

It works by processing audio in the frequency domain, allowing for unique effects like:
- **Spectral Vocoding**
- **Harmonic Filtering**
- **Frequency Shifting**
- **Spectral Smearing**

## Features
- **Universal Support**: Native compatibility for both Apple Silicon (M1/M2/M3) and Intel processors.
- **Modern GUI**: Rebuilt user interface using the JUCE framework.
- **Stereo Processing**: True stereo operation for all effects.
- **Ad-hoc Signed**: Ready for local development and use in DAWs like Ableton Live.

## Installation

1.  **Download/Build**: Ensure you have the `DtBlkFx_GUI.vst3` bundle.
2.  **Install**: Copy the plugin to your VST3 directory:
    ```bash
    cp -R build/universal/DtBlkFx_GUI.vst3 ~/Library/Audio/Plug-Ins/VST3/
    ```
3.  **Rescan**: Open your DAW (e.g., Ableton Live) and rescan for plugins.

## Building from Source

### Prerequisites
- **CMake** (3.19+)
- **Xcode** (or Command Line Tools)
- **vcpkg** (included as a submodule)

### Build Instructions

1.  **Clone the repository**:
    ```bash
    git clone --recursive https://github.com/yourusername/dtblkfx_mac.git
    cd dtblkfx_mac
    ```

2.  **Run the Universal Build Script**:
    This script handles dependency installation (via vcpkg), building for both x86_64 and arm64, creating a universal binary, and signing it.
    ```bash
    ./build_universal.sh
    ```

3.  **Output**:
    The built plugin will be located at:
    `build/universal/DtBlkFx_GUI.vst3`

## Usage
- **Mix Back**: Controls the balance between the original and processed signal.
- **Delay**: Adds a delay to the processed signal.
- **FFT Length**: Adjusts the size of the FFT window (frequency resolution vs. time resolution).
- **Overlap**: Controls the overlap of FFT windows (smoother sound vs. CPU usage).
- **Effect Parameters**:
    - **Freq A/B**: Frequency range for the effect.
    - **Amp**: Amplitude of the effect.
    - **Val**: Effect-specific parameter.

## License
This project is licensed under the GNU General Public License v2.0 (or later). See `COPYING` for details.

## Credits
- **Original Author**: Darrell Tam
- **macOS/JUCE Port**: haiori
