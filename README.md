<p align="center">
<a href="https://github.com/anomalyco/opencode"><img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++20"></a>
<a href="https://github.com/pavelc4/rfxh/releases"><img src="https://img.shields.io/github/v/release/pavelc4/rfxh?label=Release&style=for-the-badge&logo=github&logoColor=white&labelColor=222" alt="Latest Release"></a>
<a href="LICENSE"><img src="https://img.shields.io/badge/MIT-white?style=for-the-badge&logo=opensourceinitiative&logoColor=white&label=License&labelColor=222" alt="License"></a>
</p>

## About rfxh

**rfxh** is a terminal fetch tool written in modern C++20. Displays system information with a rotating 3D ASCII logo — inspired by neofetch and fastfetch, built from scratch with a modular architecture.

## Preview

<video src="docs/assets/preview.mp4" controls width="100%"></video>

## Features

- **3D ASCII Logo** — Built-in logo library with 450+ distro logos + Android (12–17, robot head) + LineageOS
- **System Info** — OS, kernel, uptime, packages, shell, display, CPU, GPU, memory, disk, battery, WM, terminal, GTK
- **Cross-Platform** — Linux, macOS, Windows (Win32), Android (Termux)
- **Built-in Logos** — No external tools required for logo display; falls back to fastfetch if available
- **Colorized Output** — Distro-specific colors with ANSI support
- **Configurable** — Custom logos, ASCII art resolution, color schemes via CLI flags

## Install

### Build from source

```bash
git clone https://github.com/pavelc4/rfxh.git
cd rfxh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/rfxh
```

Requires: Clang 18+, CMake 3.20+

### Download pre-built binary

Download the latest binary for your platform from the [Releases](https://github.com/pavelc4/rfxh/releases) page.

```bash
chmod +x rfxh-*
./rfxh-linux-x86_64
```

## Usage

See [Commands Reference](docs/commands.md) for full documentation.

```bash
rfxh                          # auto-detect distro + logo
rfxh --logo arch              # pick a specific logo
rfxh --logo android           # Android generic
rfxh --logo lineageos         # LineageOS DNA helix
rfxh --speed 2.0              # faster rotation
rfxh --size 2.0               # larger logo
rfxh --no-info                # logo only, no system info
rfxh --no-color               # monochrome logo
rfxh --infinite               # animate forever
rfxh --shading-chars '@#%+.'  # custom shading ramp
rfxh --help                   # full options
```

## Built-in Logos

| Category | Count | Examples |
|----------|-------|---------|
| Linux    | 400+  | Arch, Debian, Fedora, Ubuntu, NixOS, Gentoo… |
| macOS    | 8     | Apple, OSX, macOS variants |
| Windows  | 7     | Windows 11, 8, 95, Server 2025 |
| BSD      | 13    | FreeBSD, OpenBSD, NetBSD, Solaris… |
| Android  | 12    | Version logos (12–17), bugdroid, LineageOS, GrapheneOS |

## Resources

- [Releases](https://github.com/pavelc4/rfxh/releases) — Download latest version
- [Issues](https://github.com/pavelc4/rfxh/issues) — Report bugs

## License

rfxh is open-sourced software licensed under the [MIT License](LICENSE).
