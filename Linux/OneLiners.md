# One Line utilities

## Recusively delete empty directories
```bash
find . -type d -empty -delete
```

This recursively finds directories (`-type d`) that are empty (`-empty`) starting from the current directory (`.`) and deletes them.


```bash
find . -depth -type d -empty -delete
```
To ensure `find` removes directories that become empty after their children are deleted, use depth-first traversal. The `-depth` option processes subdirectories before their parents, which is important for fully cleaning nested empty directories in one pass.

----
