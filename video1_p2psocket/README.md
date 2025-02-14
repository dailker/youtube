If you haven't installed the environment yet, please install it if you are not on Linux or Mac:

[MSYS2 Installation](https://www.msys2.org/)

And here is a POSIX-Windows compliant library. I recommend you enhance yourself with it:
[libmingw32_extended](https://github.com/CoderRC/libmingw32_extended)

For the installation of the library, follow the steps provided by the author. It's very easy to copy-paste into MSYS or your environment.

Here are the commands adjusted for your setup:

```bash
gcc folder/client.c -o folder/client.exe -lmingw32_extended
```
```bash
gcc folder/node.c p2p2/main.c -o folder/server.exe -lmingw32_extended
```

and last :) if you having any problem just let me know on discord i here to help you ;)

https://discord.gg/FmGuAK98TR

## extra note: you can submit your adjusted code this ./example folder many people can benefit of.
