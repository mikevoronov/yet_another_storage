# yet_another_storage

## General idea

This storage supports all common operations with keys. It is based on physical volumes (PV in terms of LVM) with the following structure:

### Header
  Header contains info about version, cluster size, bin counts, priority, and offsets of all other structures.

### Inverted index
  Inverted index is based on Aho-Corasick algorithm. Its main purpose is to save info about offsets of all keys for values in data section. Since it's assumed that a PV could have a too big size to keep it all in RAM both with perfomance reqs so each PV represents in memory only with inverted index.

### Freelists
  Freelists exploits concept of fastbin of (dl)ptmalloc. In this version of library cluster size is fixed and equals 3840 bytes (to guaranteed fit at page size on x86/amd64) so there are following freelist buckets: | 4 | 8 | <16 | <32 | <64 | <128 | <256 | <512 | <1024 | <2048 | <3840 |. Freelists are also located in RAM but it requires only 4*11 bytes for each PV.
 
### Data
  Similar to common filesystems: PV is divided on clusters with fixed size (3840). And there are three types of data entry: 
  1. simple type with 4 bytes
  2. simple type with 8 bytes
  3. complex type with non-fized size
  The first two have a simple header with minimum size (12 bytes for 4 bytes type and 16 for 8). It assummed that they can't occupy several clusters. 
  The third one can occupy several clusters and to support it has a special link to next chunk with it's data.

## Dependences 

1. nonstd::expected (https://github.com/martinmoene/expected-lite) - header-only

## Installation

The VS 2017 (C++17) is required to build this lib.
