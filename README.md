# slowjs

A standalone (just libc) JavaScript interpreter.


### Example

```bash
$ cat examples/plus.js
function main() {
  return 1 + 3;
}
$ ./bin/slowjs examples/plus.js
3
```

### Build

```bash
$ mkdir build
$ cd build
$ cmake ..
```

### Test

```bash
$ mkdir test
$ cd test
$ ctest ..
```
