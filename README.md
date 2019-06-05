`tildefier` is a small utility for creating nice shell prompts. It takes a path (usually the working directory), normalizes it and writes it as a subdirectory of a user's home folder where possible:

```bash
$ cd ~user/foo
$ tildefier
~user/foo
$ whoami
user
$ tildefier --short
~/foo
$ tildefier /root
~root
$ tildefier /usr
/usr
$ tildefier symlink-to-root
/
```

## Caveats
If you are passing paths via the command line arguments, you should always use `--` to prevent paths starting with a dash to be interpreted as an option:
```bash
$ tildefier -- "$PATH"
~/--help
```

Some shells (e.g., fish) work nicely with normalized paths, as the `cd` in the resolved directory tree. Others (e.g., bash) operate on the path string itself, leading to weird behavior:
```bash
~/foo $ echo $0
bash
~/foo $ mkdir a ; mkdir b ; ln -s ../a b/a
~/foo $ tree
.
├── a
└── b
    └── a -> ../a

3 directories, 0 files
~/foo $ cd b/a
~/foo/b/a $ tildefier --short
~/foo/a
~/foo/b/a $ cd .. # bash's interpretation of the current path differs from tildefiers due to the symbolic path element
~/foo/b $ tildefier --short # since no symbolic links are part of the path anymore, their interpretation is the same again
~/foo/b
```
