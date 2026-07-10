# rfxh Commands Reference

## Preview

<video src="assets/preview.mp4" controls width="100%"></video>

## Usage

```bash
rfxh [options]
```

## Options

| Flag | Description |
|------|-------------|
| `-l, --logo <name>` | Use a specific logo by name (e.g. `arch`, `android`, `lineageos`, `bugdroid`) |
| `--rotate-x` | Lock rotation to X axis only |
| `--rotate-y` | Lock rotation to Y axis only |
| `-s, --speed <float>` | Rotation speed multiplier (default: `1.0`) |
| `--size <float>` | Scale logo size (`0.5` – `5.0`, default: `1.0`) |
| `--height <n>` | Override render height in rows |
| `--no-info` | Display logo only, hide system information |
| `--no-color` | Disable logo colorization |
| `--frames <n>` | Stop after `n` frames (default: `2000`) |
| `--infinite` | Run forever (overrides `--frames`) |
| `--shading-chars <str>` | Custom shading character ramp (UTF‑8, default: `.,-~:;=!*#$@`) |
| `-h, --help` | Show help and exit |

## Examples

### Basic usage

```bash
rfxh
```

### Custom logo

```bash
rfxh --logo android
rfxh --logo lineageos
rfxh --logo bugdroid
rfxh --logo "android 17"
rfxh --logo "ubuntu"
```

### Animation control

```bash
rfxh --speed 2.5                     # faster rotation
rfxh --rotate-x                      # rotate on X axis only
rfxh --infinite                      # run forever
rfxh --frames 500                    # stop after 500 frames
```

### Display

```bash
rfxh --size 2.0                      # larger logo
rfxh --height 40                     # force 40 rows height
rfxh --no-info                       # hide system info
rfxh --no-color                      # monochrome output
rfxh --shading-chars '@%#+.'         # custom shading
```

## Built-in Logos

| Category | Count | Lookup Keys |
|----------|-------|-------------|
| Linux    | 400+  | `arch`, `ubuntu`, `fedora`, `debian`, `nixos`, `gentoo`, `manjaro`, `void`, … |
| macOS    | 8     | `macos`, `osx`, `apple`, … |
| Windows  | 7     | `windows`, `windows 11`, `windows 95`, … |
| BSD      | 13    | `freebsd`, `openbsd`, `netbsd`, `solaris`, … |
| Android  | 12    | `android`, `android 17`, `lineageos`, `bugdroid`, `grapheneos`, … |

Use `--logo <key>` to display any logo. The lookup is case-insensitive.

## Preview

<!-- video placeholder: docs/assets/preview.mp4 -->
