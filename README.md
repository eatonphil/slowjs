# slowjs

A standalone (just libc) JavaScript interpreter.

### Example

```
$ cat examples/plus.js
function main() {
  return 1 + 3;
}
$ make
$ ./bin/slowjs examples/plus.js
3
```