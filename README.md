# VNTools

![License: GPL v2+](https://img.shields.io/badge/License-GPL%20v2%2B-blue.svg)

A collection of utilities for working with visual novel assets written in portable ISO C99.

## Overview

VNTools provides tools for handling various visual novel engine formats, allowing you to unpack, pack, and convert assets from multiple VN engines.

## Supported Engines

| Engine | Format | Capabilities |
|--------|--------|--------------|
| Cromwell | .opk | Unpacking and packing |
| Cromwell | .pak | Unpacking only |
| IkuraGDL | .GGP, SM2MPX10 | GGP decryption, SM2MPX10 archive unpacking and packing |
| Studio neko punch | - | Unpacking only |
| Studio-Sakura | - | Unpacking only |
| Succubus | - | Unpacking only |
| Silky's | .IFL | Unpacking and repacking |
| Carriere | .CGD | Conversion to BMP only |
| BasiL | .MIF | Unpacking only |
| ARCX | .arc | Unpacking (repacking not necessary, runs with loose files) |

## Contributing

Spotted an issue? Feel like contributing? Open a pull request! Contributions are welcome.

## License

Everything is licensed under GPLv2 (and LGPLv2.1) or later unless otherwise specified.
