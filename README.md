# IDNameTag

**English** | [Português](README.pt-BR.md)

An `.asi` plugin for **SA-MP** that changes player nametags from `Name (45)` to `Name (ID: 45)`. You can configure the format.

# Preview:

![Nametag preview](preview.png)

## Installation

1. Download `IDNameTag.asi` from the [Releases](../../releases) page.
2. Put it in your GTA San Andreas folder, next to `gta_sa.exe`.
3. You need an **ASI Loader**. CLEO, SilentPatch and SAMPFUNCS already include one.

`IDNameTag.ini` is created automatically the first time the game starts.

## Configuration (`IDNameTag.ini`)

```ini
[NameTag]
Format=%s (ID: %d)
Disable_ID=0
```

`%s` is the player's **name** and `%d` is their **ID**.

Set `Disable_ID=1` to hide the ID and show only the name (`LoremIpsum`). When it is on, `Format` is ignored.

| `Format=`     | Result                  |
|---------------|-------------------------|
| `%s (ID: %d)` | `LoremIpsum (ID: 45)` |
| `%s [%d]`     | `LoremIpsum [45]`     |
| `%s #%d`      | `LoremIpsum #45`      |

Rules:

- `%s` must come **before** `%d`, and each one appears only once.
- The format can be at most **24 characters** long.
- Use `%%` for a literal `%`.
- If the format is invalid, the plugin falls back to the default `%s (ID: %d)`. This keeps the game from crashing.

The `.ini` is only read when the game starts.

## How it works

`samp.dll` builds the nametag text with `sprintf(buf, "%s (%d)", name, id)`. The plugin looks for that string in `samp.dll`'s memory. It then changes the `push` instructions that point to it so they point to the configured format.

The plugin doesn't use hardcoded addresses, so it should work on several client versions (0.3.7 R1/R3/R4/R5, 0.3.DL). For diagnostics, use [DebugView](https://learn.microsoft.com/sysinternals/downloads/debugview). It shows whether the string was found and how many references were patched.

> Some server anticheats check for changes to `samp.dll`. This change is cosmetic only, but check the server's rules before using it.

## Building

Requirements: Visual Studio with the **"Desktop development with C++"** workload.

```bat
build.bat
```

The output is `out\IDNameTag.asi`. It is always built as 32-bit.


## Author:

**[Shazanxz](https://github.com/Shazanxz)**

## License

[MIT](LICENSE)
