# yet_another_storage

This storage supports all common operations with keys. It based on physical volumes (PV in terms of LVM) with the following structure:

### Header
  Header contains info about version, cluster size, priority, and offsets of all other structures.

### Inverted index
  Inverted index based on Aho-Corasick algorithm. Its main purpose is to save info about offsets all keys in data Section. Since it's assumed that a PV could have a too big size to keep it all in RAM both with perfomance reqs so each PV reprents in memory only with inverted index.

### Freelists
  Freelists exploits concept of fastbin in (dl)ptmalloc. In this version of library cluster size is fixed and equals 3840 bytes (to guaranteed fit at page size on x86/amd64) so there are following freelist buckets: | 4 | 8 | <16| <32| <64| <128| <256| <512 | <1024| <2048| <3840|. Freelists are also located in RAM but it requires only 4*bucket_count bytes.
 
### Data
  Similar with FAT filesystem: all data divided to clusters with fized size. There are 3 types of data entry: simple type with 4 bytes, simple type with 8 bytes and complex type with non-fized size. The first two have a simple header with minimum size. It assummed that they cann't occupy several clusters. The third one can occupy several clusters and has a special link to next chunk with it's data.


## Dependences 

1. nonstd::expected (https://github.com/martinmoene/expected-lite) - header-only
2. boost::interprocess::mapped_file
3. boost::test
