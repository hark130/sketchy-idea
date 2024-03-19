# sketchy-idea
Reusable, stand-alone, Linux code

## SKETCHY IDEA (SKID)

My opportunity to write some clean Linux code that is tested and releasable.

## DEPENDENCIES

* [GNU Make](https://www.gnu.org/software/make/) - Underpins the build system
* [GCC](https://gcc.gnu.org/) - The compiler
* [Check](https://github.com/libcheck/check) - A unit test framework for C

## USAGE

### Do Everything Everywhere All At Once

`make`

### Is everything installed properly?

`make validate`

### Are all the Check unit tests *really* passing?

```
make && \
for check_bin in $(ls code/dist/check_*.bin); do $check_bin && CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all $check_bin; done
```
