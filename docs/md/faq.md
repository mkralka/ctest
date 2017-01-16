# FAQ

## Defining Suites
### Can I have more than one test suite in a single translation unit (source file)?

> No. Just like it's unwise to combine unrelated code into a single translation
> unit, it's unwise to combining unrelated unit tests in a single translation
> unit. A good rule to follow is that each source file should have a
> corresponding unit test file that tests all of the functionality in the
> source files. These should belong to a single suite.

### Can I have multiple suites in a unit test module?

> No. Each suite should be located in its own module. Tools for running tests
> support multiple modules, so multiple suites can be run together.
> Furthermore, to maximize portability, ctest avoided features that are not
> available on most UNIX-like systems, making it difficult to support multiple
> suites per module (without a lot of hacking).
