# Basic Package for Zenith

A comprehensive helper package for the Zenith programming language, providing essential utilities for testing, I/O operations, and common programming tasks.

## Package Structure

```
package/
├── package.zen              # Main package file
├── basic.zen                # Package root module
├── testing/
│   ├── testing.zen          # Unit testing utilities
│   └── integration.zen      # Integration testing utilities
├── utils/
│   └── utils.zen            # General utility functions
├── io/
│   └── io.zen               # Input/Output utilities
└── examples/
    ├── testing_example.zen  # Testing module example
    ├── utils_example.zen    # Utils module example
    └── io_example.zen       # IO module example
```

## Installation

Copy the `package` folder to your Zenith project directory or add it to your Zenith path.

## Usage

### Importing the Package

```zenith
import basic.testing.{assert_equal, assert_true, run_suite, print_summary};
import basic.utils.{abs, min, max, clamp, is_even};
import basic.io.{file_read_all, file_write, file_exists};
```

### Testing Module

The testing module provides comprehensive unit and integration testing capabilities:

```zenith
import basic.testing.{assert_equal, run_test, print_summary};

fn my_test() -> Bool {
    return assert_equal(2 + 2, 4, "Math should work");
}

fn main() {
    let result = run_test("My Test", my_test);
    println("Test: " + result.name + " - " + result.message);
}
```

**Features:**
- `assert_true(condition, message)` - Assert that a condition is true
- `assert_equal(actual, expected, message)` - Assert equality for integers
- `assert_string_equal(actual, expected, message)` - Assert equality for strings
- `run_test(name, test_fn)` - Run a single test function
- `run_suite(suite_name, tests)` - Run a collection of tests
- `print_summary(suite)` - Print test results summary

### Utils Module

General purpose utility functions for everyday programming:

```zenith
import basic.utils.{abs, min, max, clamp, is_even, array_contains};

fn main() {
    println("Absolute value: " + abs(-5));  // 5
    println("Min: " + min(10, 20));         // 10
    println("Max: " + max(10, 20));         // 20
    println("Clamped: " + clamp(15, 0, 10)); // 10
    println("Is even: " + is_even(4));      // true
}
```

**Categories:**
- **String Utilities**: `is_empty`, `to_uppercase`, `to_lowercase`, `trim`, `contains`, `replace`, `split`, `join`
- **Numeric Utilities**: `abs`, `min`, `max`, `clamp`, `is_even`, `is_odd`
- **Array Utilities**: `array_length`, `array_contains`, `array_index_of`, `array_reverse`, `array_sort`, `array_unique`
- **Boolean Utilities**: `all`, `any`, `negate`, `xor`

### IO Module

File operations, console I/O, and data serialization:

```zenith
import basic.io.{file_read_all, file_write, file_exists, json_parse};

fn main() {
    // File operations
    file_write("test.txt", "Hello, Zenith!");
    let content = file_read_all("test.txt");
    println(content);
    
    // JSON parsing
    let json = json_parse("{\"name\": \"Zenith\"}");
    if (json.is_valid) {
        println("Valid JSON!");
    }
}
```

**Features:**
- **File Operations**: `file_open`, `file_close`, `file_read_all`, `file_write`, `file_append`, `file_exists`, `file_delete`, `file_copy`, `file_size`
- **Directory Operations**: `dir_create`, `dir_exists`, `dir_list`, `get_current_dir`, `set_current_dir`
- **Console I/O**: `read_line`, `read_int`, `read_float`, `console_clear`, `console_color`
- **Data Serialization**: `json_parse`, `json_stringify`, `csv_parse`, `csv_generate`
- **Path Utilities**: `path_join`, `path_dirname`, `path_basename`, `path_extension`, `path_normalize`

## Examples

Run the example files to see the package in action:

```bash
zenith package/examples/testing_example.zen
zenith package/examples/utils_example.zen
zenith package/examples/io_example.zen
```

## API Reference

### Testing Structures

```zenith
struct TestResult {
    name: String,
    passed: Bool,
    message: String,
    duration: Int
}

struct TestSuite {
    name: String,
    total: Int,
    passed: Int,
    failed: Int,
    results: [TestResult]
}
```

### IO Structures

```zenith
struct File {
    path: String,
    mode: String,
    is_open: Bool,
    handle: Int
}

struct JsonValue {
    data: String,
    is_valid: Bool
}

struct CsvRow {
    columns: [String]
}
```

## Contributing

Contributions are welcome! Please follow the Zenith coding standards and ensure all tests pass before submitting.

## License

This package is part of the Zenith programming language ecosystem.

## Version

**1.0.0** - Initial release
