# sketchy-idea
Reusable, stand-alone, Linux code.

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

The for loop below will execute all the check_*.bin files and then execute them again with Valgrind.
The for loop will stop if any failure is encountered: Check, Valgrind, or otherwise.

```
make && \
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break || CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 $check_bin; [[ $? -ne 0 ]] && break || continue; done
```

### How many Check unit tests *are* there?

```
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}'
```

## NOTES

### Save the output

```
date > output.txt && echo >> output.txt && \
make >> output.txt && echo >> output.txt && \
for check_bin in $(ls code/dist/check_*.bin); do CK_FORK=no valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 $check_bin >> output.txt 2>&1; [[ $? -ne 0 ]] && break || continue; done && echo >> output.txt && \
for check_bin in $(ls code/dist/check_*.bin); do $check_bin; [[ $? -ne 0 ]] && break; done | grep "100%: Checks: " | awk '{sum += $3} END {print "TOTAL CHECK UNIT TESTS: "sum}' >> output.txt
```

### Check unit test framework

* Convient function-like [test macros](https://libcheck.github.io/check/doc/check_html/check_4.html#Convenience-Test-Functions)
* [Test Fixtures](https://libcheck.github.io/check/doc/check_html/check_4.html#Test-Fixtures)
