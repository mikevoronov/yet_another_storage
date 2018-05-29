# yet_another_storage

## Basic defenitions
Physical volume (PV) - is a single physical storage unit located anywhere which is could be accessed by special device class.
Catalog - is a any prefix substring from prefix trie of keys from PV. F.e. assume that we have 3 keys: "/home/user1/file1", "/home/user1/file2", "/home/user2/file". So catalog is any prefix from prefix trie build on these keys. Since if we have choosen "/home/us" (or any prefix substring of "/home/user") catalog that we have an access to all of these keys.

## PV layout

Each PV has following layout on :

### PVHeader
  Header contains info about version, cluster size, bin counts, priority, and offsets of all other structures.

### Freelists
  Freelists exploits the same concept as fastbin in (dl)ptmalloc. In version 1.1 of library cluster size could be choosen arbitrarily but by default equals 3840 bytes (to guaranteed fit at page size on x86/amd64). Because of this there are following freelist buckets | 12 | 16 | <64 | <100 | <128 | <256 | <512 | <1024 | <1520 | <2048 | <3840 | have been chosen.
 
### Inverted index
  The main purpose of inverted index in PV is to keep info (mainly offsets) for all keys. In version 1.1 inverted index is based on Aho-Corasick algorithm. In PV it is saved in serialized view.

### Data
  Similar to common filesystems: PV is divided on clusters with choosen size and has several types of data entry. For version 1.1 there are 3 types:
  1. "simple" 4 bytes type,
  2. "simple" 8 bytes type,
  3. "complex" type with non-fixed size.

  The first two have a simple header with minimum size (12 bytes and 16 bytes respectively). It is assummed that these types of headers can't occupy several clusters. 
  The third types can occupy several clusters and so has a special structure with link to next chunk (single linked list). Each headers could be in 2 states: allocated and freed. In the first state it simply contains user's data but in the second state in place of data it has a link to the next freed chunk of corresponded size (see section about Freelists). The begining of this list always located at freelist bins.

## Storage
Storage is a some kind of virtual PV. It is also based on the same inverted index as PV. Storage support mounting of any catalog of PV to any virtual Storage catalog. Also it is possible to mounting any several catalogs of one PV to several catalogs of Storage. In case of path ambiguity operations are performed on the PV with the hightest priority (priority specifies by user while creating PV).
 
## General idea
Since it's assumed that a PV could have a too big size to keep it all in RAM each PV represents in memory only by inverted index and freelists bins. To minimize memory overhead inverted index save only offset of data entry in PV. All other information (type and expired date) about key is located in the entry header. So all common operation on PV suggest prefix index traversal to find a offset of value PV, then determine entry types (simple4, simple8 or complex) and expired status and then execution the operation itself. For write new entry to PV it is used freelist bins. In version 1.1 of library all freed entries is located in several single linked lists and freelist helper provides with capability to provide a most sutable chunk. If there aren't any freed chunk in list PV is physically expanded to several cluster size (in current version by 5 cluster size at once). So in general case it isn't need to long traversal through PV strucutre with a lot of i/o operation.

Library is highly template and now supports unicode, working in 32/64 bit (in 32 bit size of PV is limited by 2**32 bytes but it implies less overhead) and heterogeneous device with the assumption about possbility only to open/read/write/close data.

At the moment YAS support following types:
- int8_t
- uint8_t
- int16_t
- uint16_t
- int32_t
- uint32_t
- float (assumed 4 bytes at size)
- int64_t
- uint64_t
- double
- std::string
- std::vector


## Dependences 

1. nonstd::expected (https://github.com/martinmoene/expected-lite) - header-only
2. gtest (https://github.com/google/googletest) for test

## Currently platform supported
On version 1.1 there is only support Windows and can be compiled by Visual C++ complier from VS 2017 (C++17). But it isn't difficult to transfer it to *nix-like OS.
