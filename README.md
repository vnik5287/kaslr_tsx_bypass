# Linux kASLR (Intel TSX/RTM) bypass static library

Uses Intel TSX/RTM (Restricted Transactional Memory) cache side-channel to get
the kernel offset.

## Usage

Link libkaslr.a to your exploit and call `get_kaslr_offset()` to get the
offset. The return value is either the kernel offset or (uint64_t)-1 on error.

```
$ gcc example.c libkaslr.a -static -lm -lpthread
```

Might try running `get_kaslr_offset()` in a loop to make sure the return value
is stable.

https://asciinema.org/a/UQhuoTWP3ZtfPbPMR69OLSK1f
