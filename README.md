# ebook-fit-to-height

[![rmppmove](https://img.shields.io/badge/rMPPMove-supported-green)](https://remarkable.com/products/remarkable-paper/pro-move)

A xovi extension that makes ebooks rendered on another reMarkable or older software version fill the screen of a reMarkable Paper Pro Move.

When an EPUB is first opened on a 3:4 device (or any device on 3.27 or lower), xochitl renders it to a 3:4 PDF. When opened on a Move on 3.28 or higher, it opens fit-to-width, which results in letterboxing.

This extension restores the 3.27 view: ebooks fit the screen height. Nothing is written to the book, so it keeps its layout on your other devices.

## Dependencies

- [xovi](https://github.com/asivery/rm-xovi-extensions) - Extension framework

## Installation

### Vellum

```
vellum add ebook-fit-to-height
```

### Manual

1. Ensure xovi is installed
2. Download `ebook-fit-to-height.so` from the [latest release](https://github.com/rmitchellscott/rm-ebook-fit-to-height/releases/latest) and place it in `/home/root/xovi/extensions.d/` on your reMarkable Paper Pro Move
3. Restart xovi

## License

Copyright (C) 2026 Mitchell Scott

Licensed under the GNU General Public License v3.0.
