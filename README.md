# aurora

__Warning: This is my personally used, very quickly and hackily made testing framework for my compiler; it is not stable at all.__

A language-independent file-based general purpose testing framework for testing Black-Boxes.

It consists of a single file and is less than one hundred lines of C code. After downloading the code compile it and install it. To do this move it to your computers folder for binaries (such as `/usr/local/bin`) to access it globally.

To use it in a particular project of yours create a file aurora_config.toml in your folder.
This file looks like this:
```
[a-test-suite]                   # name of the test suite - whitespace is not allowed
exec = "some-exec" # the  executable to run on every file
test_folder = "tests"            # the tests folder
```
Also create a folder tests (or name it anything else as long as it lines up with the `test_folder` definition).
For example tests might look like this:
```
tests                                                                                    
├── a-test-file.some
└── tests.toml
```
and tests.toml like this:
```
[my first test]
file = "a-test-file.some" # the file to run the executable on
expect = "blabla"         # expected output
return = 1                # and expected return code
skip = true               # if this flag is set to true the test is skipped
```

Aurora works by running the exe on all the files specified in the the tests.toml and checking wether their outputs match the requirements.
If all goes right and your code works, aurora happily outputs "Congratulations all tests passed! :)"
