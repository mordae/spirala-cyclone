# Krecek

## Development on PC

```bash
# Install dependencies
sudo dnf install SDL2-devel SDL2_Pango-devel pulseaudio-libs-devel libasan

# Prepare the project in Debug mode with address sanitizer
cmake -B build/host host -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build the project for host
cmake --build build/host

# Ask address sanitizer to ignore allocation issues.
export ASAN_OPTIONS='detect_leaks=0'

# Run a game
build/host/pong/pong
```
