# syslog-redirector

The project implements LD_PRELOADable wrapper library that allows output of `syslog()` call to be redirected to any given UNIX domain socket instead of the hardcoded path in the C library.
It is useful for example when running in a container, where the `/dev/log` socket is forwarded to the host, but you want to keep the logs inside the container.

The heavy lifting is done by the `syslog.c` file from [musl libc](https://www.musl-libc.org/), which is licensed under MIT license.

## Usage

Build the library:

```
make
```

Then run your program with `LD_PRELOAD` set to the path of the library and `SYSLOG_SOCKET` set to the path of the socket you want to redirect the logs to:

```
LD_PRELOAD=/path/to/libsyslog-redirector.so SYSLOG_SOCKET=/path/to/socket ./your-program
```
