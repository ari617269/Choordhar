# Choordhar

A C++ implementation of a **typed TLV (Tag–Type–Length–Value) serialization and deserialization system**, built as a learning-focused exploration of binary protocols, data encoding, and forward/backward compatibility.

This repository demonstrates how to design, implement, and reason about a self-describing binary wire format without relying on fixed offsets or rigid struct layouts.

---

## Summary

This project implements a minimal yet extensible **TLTV (Tag-Type-Length-Value)** format in C++.

### Key ideas
- **Tags identify meaning**, not position
- **Types define how bytes are interpreted**
- **Lengths allow safe skipping of unknown fields**
- No decoding logic depends on struct order or byte offsets
- Forward and backward compatible by design

### Supported features
- Primitive types: `INT (int64)`, `BOOL`, `STRING`
- Composite types: `ARRAY` (nested TLTV entries)
- Big-endian encoding for portability
- Safe parsing with truncation checks
- Round-trip serialize → deserialize example (`MyData`)

The implementation evolves from a minimal prototype to a structured design that can be extended into a reusable library.

---

## TLTV Wire Format

Each entry is encoded as:

```bash
[ Tag : 2 bytes (big-endian) ]
[ Type : 1 byte ]
[ Length : 4 bytes (big-endian) ]
[ Value : Length bytes ]

```

- Arrays and objects contain **nested TLTV entries** in their Value region.

## Setup

### Requirements
- C++17 or later
- Any standard compiler (`g++`, `clang++`, MSVC)

### Clone the repository
```bash
git clone https://github.com/ari617269/Choordhar.git
cd Choordhar
```
