---
id: noreturn
title: T.noreturn
---

> TODO: This page is still a fragment. Contributions welcome!

```
T.noreturn
```

This indicates that a method never returns (for instance, it might loop
infinitely, raise an exception, or exit the program). A method that
unconditionally raises might be typed as `returns(T.noreturn)`.

... might want to mention something about how `T.noreturn` powers dead code
checking internally?
