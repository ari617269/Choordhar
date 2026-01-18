# Choordhar

A C++ implementation of a **typed TLV (Tag–Type–Length–Value) serialization and deserialization system**, built as a learning-focused exploration of binary protocols, data encoding, and forward/backward compatibility.

This repository demonstrates how to design, implement, and reason about a self-describing binary wire format without relying on fixed offsets or rigid struct layouts.

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
chmod +x build.sh
./build.sh
```

###Run
####Compile
```bash
g++ -std=c++17 tltv.cpp -o tltv
```

####Execute
```bash
./tltv
```

###Expected output

- Hex dump of the serialized TLTV buffer
- Deserialized values printed to stdout
- Successful round-trip verification of:
  - bool
  - int64
  - string
  - array<int64>
