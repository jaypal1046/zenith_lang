# Variables and Types in Zenith

Understanding data types is fundamental to programming in Zenith. This guide covers all built-in types and how to use them.

## Basic Types

### Numbers

Zenith supports two numeric types:

```zenith
// Integer (whole numbers)
let age: Int = 25;
let count: Int = -10;
const MAX_VALUE: Int = 1000;

// Double (decimal numbers)
let price: Double = 19.99;
let pi: Double = 3.14159;
const GRAVITY: Double = 9.81;
```

### Strings

Strings are used for text:

```zenith
let name: String = "Alice";
let greeting: String = 'Hello, World!';

// String interpolation
let age: Int = 30;
print("My name is $name and I'm $age years old");

// Multi-line strings
let description: String = """
This is a multi-line
string that spans
multiple lines.
""";
```

### Booleans

Boolean values represent true or false:

```zenith
let isActive: Bool = true;
let hasPermission: Bool = false;

// Boolean operations
if isActive && hasPermission {
  print("Access granted");
}

if !isActive || hasPermission {
  print("Special case");
}
```

## Collections

### Lists

Lists are ordered collections of elements:

```zenith
// List of integers
let numbers: List<Int> = [1, 2, 3, 4, 5];

// List of strings
let names: List<String> = ["Alice", "Bob", "Charlie"];

// Empty list
let empty: List<String> = [];

// List operations
numbers.add(6);
numbers.remove(1);
let length: Int = numbers.length;

// Access elements
let first: Int = numbers[0];
let last: Int = numbers[-1]; // Negative indexing

// List comprehension
let squares: List<Int> = [x * x for x in numbers];
```

### Maps

Maps are key-value pairs:

```zenith
// Map with string keys and int values
let ages: Map<String, Int> = {
  "Alice": 25,
  "Bob": 30,
  "Charlie": 35,
};

// Access values
let aliceAge: Int = ages["Alice"] ?? 0;

// Add/update entries
ages["David"] = 40;

// Check if key exists
if ages.containsKey("Bob") {
  print("Bob is in the map");
}

// Iterate over map
for (key, value) in ages.entries {
  print("$key is $value years old");
}
```

## Type Inference

Zenith can infer types automatically:

```zenith
// Type is inferred as Int
let count = 42;

// Type is inferred as String
let message = "Hello";

// Type is inferred as List<Int>
let numbers = [1, 2, 3];

// Explicit type annotation (recommended for clarity)
let explicit: Int = 42;
```

## Null Safety

Zenith has null safety built-in:

```zenith
// Nullable type (can be null)
let name: String? = null;

// Non-nullable type (cannot be null)
let title: String = "Default"; // This will never be null

// Null-aware operators
let length: Int = name?.length ?? 0; // If name is null, use 0

// Null assertion (use when you're sure it's not null)
let safeLength: Int = name!.length;
```

## Custom Types

You can define your own types:

```zenith
class Person {
  let name: String;
  let age: Int;
  
  constructor(name: String, age: Int) {
    this.name = name;
    this.age = age;
  }
  
  fun greet() -> String {
    return "Hi, I'm $name";
  }
}

// Usage
let person = Person("Alice", 25);
print(person.greet());
```

## Type Conversion

Convert between types:

```zenith
// String to Int
let numStr: String = "42";
let num: Int = Int.parse(numStr);

// Int to String
let count: Int = 100;
let str: String = count.toString();

// Double to Int (truncates)
let pi: Double = 3.14;
let whole: Int = pi.toInt(); // 3

// Int to Double
let wholeNum: Int = 5;
let decimal: Double = wholeNum.toDouble(); // 5.0
```

## Constants

Use `const` for values that never change:

```zenith
const PI: Double = 3.14159;
const MAX_USERS: Int = 1000;
const APP_NAME: String = "MyApp";

// Compile-time constants
const SECONDS_PER_MINUTE: Int = 60;
const MINUTES_PER_HOUR: Int = 60;
const SECONDS_PER_HOUR: Int = SECONDS_PER_MINUTE * MINUTES_PER_HOUR;
```

## Best Practices

1. **Use explicit types** for function parameters and return values
2. **Prefer `const`** for values that don't change
3. **Use meaningful names** (camelCase for variables, PascalCase for classes)
4. **Avoid nullable types** unless necessary
5. **Use type inference** for local variables when obvious

## Examples

### Complete Example

```zenith
fun main() {
  // Basic types
  let name: String = "Alice";
  let age: Int = 25;
  let height: Double = 5.7;
  let isActive: Bool = true;
  
  // Collections
  let hobbies: List<String> = ["reading", "coding", "gaming"];
  let scores: Map<String, Int> = {"math": 95, "science": 88};
  
  // Print information
  print("Name: $name");
  print("Age: $age");
  print("Height: $height ft");
  print("Active: $isActive");
  print("Hobbies: $hobbies");
  print("Math Score: ${scores["math"]}");
}
```

## Next Steps

- [Functions](03-functions.md) - Learn to write reusable code
- [Control Flow](04-control-flow.md) - Conditionals and loops
- [Classes](05-classes.md) - Object-oriented programming

## Need Help?

- [Language Specification](../docs/language-spec.md)
- [API Reference](../api/types.md)
- [Examples](https://github.com/zenith-lang/examples)
