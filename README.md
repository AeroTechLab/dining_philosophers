# Dining Philosophers Problem

### Embbeded Systems SEM-0544
##### Brazil, São Carlos, EESC - USP

A repository with some implementations of the Dining Philosophers Problem

### `problem_with_mutex` and `problem_without_mutex`

`problem_with_mutex` and `problem_without_mutex` were developed by the repository owner

Both of the above implementations may be compiled using CMake or the following:

```
gcc problem_with_mutex.c -o problem_with_mutex -lpthread
gcc problem_without_mutex.c -o problem_without_mutex -lpthread
```

To compile with debug symbols, just append `-g` to the compilation command

In each project, a file with a log file for each thread is created in the current folder (`logPhil0.txt`, `logPhil1.txt`, `logPhil2.txt` etc)

All these files may be fusion using the script `analyser.py` by running it in the log folder

### `problem_threadless` and `problem_with_sempahore_bad` 

`problem_threadless` and `problem_with_sempahore_bad` were extracted from sites (each is shown in the begining of respectives source files)


Both of the above may be compiled using CMake or the following:

```
gcc problem_threadless.c -o problem_threadless
gcc problem_with_sempahore_bad.c -o problem_with_sempahore_bad -lpthread
```
