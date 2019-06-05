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
