# CI Build Dependencies

Complete list of system dependencies required to build echoform with QA harness.

## Ubuntu/Debian

```bash
sudo apt-get update && sudo apt-get install -y \
  libasound2-dev \
  libx11-dev \
  libxext-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libfreetype6-dev \
  libfontconfig1-dev \
  libcurl4-openssl-dev \
  liblapack-dev \
  liblapacke-dev \
  libopenblas-dev \
  libfftw3-dev
```

## macOS

```bash
# Xcode Command Line Tools (should be installed)
xcode-select --install

# No additional dependencies needed - frameworks included
```

## Dependency Breakdown

| Library | Purpose | Required By |
|---------|---------|-------------|
| `libasound2-dev` | ALSA audio | JUCE |
| `libx11-dev`, `libxext-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev` | X11 windowing | JUCE (juceaide) |
| `libfreetype6-dev`, `libfontconfig1-dev` | Font rendering | JUCE |
| `libcurl4-openssl-dev` | HTTP/HTTPS networking | JUCE |
| `liblapack-dev`, `liblapacke-dev` | Linear algebra | SAF (room simulation) |
| `libopenblas-dev` | BLAS implementation | SAF |
| `libfftw3-dev` | FFT library | SAF |

## Testing Locally

Before pushing to CI, test the exact build configuration:

```bash
cd /Users/artbox/Documents/Repos/echoform

# Clean build
rm -rf build_qa

# Configure (exact CI command)
cmake -B build_qa \
  -DENABLE_QA_HARNESS=ON \
  -DBUILD_QA_TESTS=ON \
  -DENABLE_TESTS=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5

# Build
cmake --build build_qa --target echoform_qa -j$(sysctl -n hw.ncpu)

# Run
cd build_qa
./echoform_qa ../scenarios/echoform_critical_suite.json
```

## Docker Test Environment (Optional)

To perfectly match CI, use Docker:

```bash
# Create Dockerfile.ci
cat > Dockerfile.ci <<'EOF'
FROM ubuntu:latest
RUN apt-get update && apt-get install -y \
  build-essential cmake git \
  libasound2-dev libx11-dev libxext-dev libxrandr-dev \
  libxinerama-dev libxcursor-dev libfreetype6-dev \
  libfontconfig1-dev liblapack-dev liblapacke-dev \
  libopenblas-dev libfftw3-dev
WORKDIR /workspace
EOF

# Build and run
docker build -t echoform-ci -f Dockerfile.ci .
docker run -v $(pwd):/workspace echoform-ci bash -c "
  cmake -B build_qa -DENABLE_QA_HARNESS=ON \
    -DBUILD_QA_TESTS=ON -DENABLE_TESTS=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 && \
  cmake --build build_qa --target echoform_qa
"
```

## Troubleshooting

### Missing Libraries

If you see `NOTFOUND` errors, check which library is missing:

```bash
# Search for package
apt-cache search <library-name>

# Install
sudo apt-get install <package>-dev
```

### Submodule Issues

```bash
# Reinitialize submodules
git submodule update --init --recursive

# Or clone fresh
git clone --recursive https://github.com/joshband/echoform.git
```
