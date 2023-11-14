# syslog-redirector

The project implements small LD_PRELOADable wrapper library that allows output of `syslog()` and `vsyslog()` calls to be redirected to any file or UNIX domain socket instead of the hardcoded path in the C library function.
The library can be useful when running an applications in a container, where the application is not allowed to write to `/dev/log` directly and when it is not feasible to modify the application code itself.

The heavy lifting is done by `syslog.c` borrowed from [musl libc](https://www.musl-libc.org/).
It is licensed under MIT license.

## Usage

Run the application with `LD_PRELOAD` set to the path of the library and `SYSLOG_PATH` set to the path to UNIX domain socket by using path `unix:/path/to/socket`

```
LD_PRELOAD=/path/to/libsyslog-redirector.so SYSLOG_PATH=unix:/path/to/socket ./your-program
```

The path can be absolute or relative to the current working directory.
Alternatively the path can be an ordinary file by using path `file:/path/to/file` or just `/path/to/file`:

```
LD_PRELOAD=/path/to/libsyslog-redirector.so SYSLOG_PATH=file:/path/to/file ./your-program
```

File can be `/dev/stdout` or `/dev/stderr` to redirect to standard output or standard error:

```
LD_PRELOAD=/path/to/libsyslog-redirector.so SYSLOG_PATH=/dev/stdout ./your-program
```


## Building

To build the library run:

```
make
```
