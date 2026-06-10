# Functions in Zenith

Functions are the building blocks of reusable code in Zenith. This guide covers everything from basic functions to advanced patterns.

## Basic Function Syntax

### Defining Functions

```zenith
// Simple function with no parameters
fun sayHello() {
  print("Hello, World!");
}

// Function with parameters
fun greet(name: String) {
  print("Hello, $name!");
}

// Function with return type
fun add(a: Int, b: Int) -> Int {
  return a + b;
}

// Function with multiple parameters
fun createUser(name: String, age: Int, email: String?) -> User {
  return User(name, age, email);
}
```

### Calling Functions

```zenith
// Call without parameters
sayHello();

// Call with positional arguments
greet("Alice");

// Call with named arguments (improves readability)
greet(name: "Bob");

// Call and capture return value
let sum: Int = add(5, 3);
print("Sum: $sum"); // Output: Sum: 8
```

## Parameters

### Required Parameters

```zenith
fun multiply(a: Int, b: Int) -> Int {
  return a * b;
}

// Must provide both parameters
let result: Int = multiply(4, 5);
```

### Optional Parameters

```zenith
// Default parameter values
fun greetUser(name: String, greeting: String = "Hello") {
  print("$greeting, $name!");
}

greetUser("Alice");           // Hello, Alice!
greetUser("Bob", "Hi");       // Hi, Bob!
```

### Named Parameters

```zenith
fun createRectangle(width: Int, height: Int, color: String = "white") -> Rectangle {
  return Rectangle(width, height, color);
}

// Use named parameters for clarity
let rect = createRectangle(
  width: 100,
  height: 200,
  color: "blue"
);

// Can skip optional parameters
let rect2 = createRectangle(
  width: 50,
  height: 75
);
```

### Variable-Length Arguments (Varargs)

```zenith
// Accept variable number of arguments
fun sumAll(numbers: Int...) -> Int {
  let total: Int = 0;
  for num in numbers {
    total += num;
  }
  return total;
}

let result1: Int = sumAll(1, 2, 3);        // 6
let result2: Int = sumAll(1, 2, 3, 4, 5);  // 15

// Spread operator
let nums: List<Int> = [1, 2, 3];
let result3: Int = sumAll(...nums);        // 6
```

## Return Types

### Single Return Value

```zenith
fun square(x: Int) -> Int {
  return x * x;
}

// Implicit return for single expressions
fun cube(x: Int) -> Int => x * x * x;
```

### Multiple Return Values

```zenith
fun divide(a: Int, b: Int) -> (Int, Int) {
  let quotient: Int = a / b;
  let remainder: Int = a % b;
  return (quotient, remainder);
}

// Destructure return values
let (q, r) = divide(10, 3);
print("Quotient: $q, Remainder: $r");
```

### Nullable Returns

```zenith
fun findUser(id: Int) -> User? {
  // Might return null if user not found
  if id < 0 {
    return null;
  }
  return User(id, "Unknown");
}

// Handle nullable return
let user: User? = findUser(42);
if user != null {
  print(user.name);
}

// Or use null-aware operator
print(user?.name ?? "User not found");
```

## Function Types

### Function as Variables

```zenith
// Define function type
type MathOp = fun(Int, Int) -> Int;

// Assign function to variable
let add: MathOp = (a, b) => a + b;
let subtract: MathOp = (a, b) => a - b;

// Use function variable
let result: Int = add(5, 3);  // 8
```

### Higher-Order Functions

```zenith
// Function that takes another function as parameter
fun calculate(a: Int, b: Int, operation: fun(Int, Int) -> Int) -> Int {
  return operation(a, b);
}

// Pass function as argument
let sum: Int = calculate(10, 5, (x, y) => x + y);
let product: Int = calculate(10, 5, (x, y) => x * y);
```

### Lambda Expressions

```zenith
// Short lambda syntax
let double = (x: Int) => x * 2;

// Multi-statement lambda
let complex = (x: Int) {
  let squared: Int = x * x;
  return squared + 1;
};

// Use with collections
let numbers: List<Int> = [1, 2, 3, 4, 5];
let doubled: List<Int> = numbers.map((n) => n * 2);
let evens: List<Int> = numbers.filter((n) => n % 2 == 0);
```

## Arrow Functions

For simple functions, use arrow syntax:

```zenith
// Traditional function
fun add(a: Int, b: Int) -> Int {
  return a + b;
}

// Arrow function (equivalent)
fun add(a: Int, b: Int) -> Int => a + b;

// Type inference with arrows
let multiply = (a: Int, b: Int) => a * b;
```

## Closures

Functions can capture variables from their scope:

```zenith
fun createCounter() -> fun() -> Int {
  var count: Int = 0;
  
  return () {
    count += 1;
    return count;
  };
}

let counter = createCounter();
print(counter());  // 1
print(counter());  // 2
print(counter());  // 3
```

## Async Functions

Zenith has built-in async/await support:

```zenith
// Async function
fun fetchData(url: String) async -> Data {
  let response = await http.get(url);
  return response.json();
}

// Call async function
fun loadData() async {
  let data = await fetchData("https://api.example.com/data");
  print(data);
}

// Multiple async operations
fun loadAllData() async -> List<Data> {
  let [users, posts, comments] = await Future.wait([
    fetchData("/users"),
    fetchData("/posts"),
    fetchData("/comments")
  ]);
  
  return [users, posts, comments];
}
```

## Generics

Functions can work with any type:

```zenith
// Generic function
fun identity<T>(value: T) -> T {
  return value;
}

// Usage with different types
let num: Int = identity<Int>(42);
let str: String = identity<String>("Hello");

// Type inference
let inferred = identity("World");  // Inferred as String
```

## Extension Functions

Add methods to existing types:

```zenith
// Add method to String
extension String {
  fun reverse() -> String {
    return this.split("").reversed().join("");
  }
  
  fun capitalize() -> String {
    return this[0].toUpperCase() + this.substring(1);
  }
}

// Usage
let name: String = "alice";
print(name.capitalize());  // "Alice"
print(name.reverse());     // "ecila"
```

## Best Practices

1. **Keep functions small** - Each function should do one thing well
2. **Use descriptive names** - Function names should explain what they do
3. **Document parameters and returns** - Use doc comments
4. **Prefer pure functions** - Avoid side effects when possible
5. **Use type annotations** - Make types explicit for clarity
6. **Handle errors gracefully** - Use Result types or exceptions

## Examples

### Complete Example

```zenith
// Calculator with various function types

fun add(a: Int, b: Int) -> Int => a + b;

fun subtract(a: Int, b: Int) -> Int => a - b;

fun multiply(a: Int, b: Int) -> Int => a * b;

fun divide(a: Int, b: Int) -> Double? {
  if b == 0 {
    return null;  // Cannot divide by zero
  }
  return a.toDouble() / b.toDouble();
}

// Higher-order function
fun calculate(a: Int, b: Int, operation: fun(Int, Int) -> Any?) -> Any? {
  return operation(a, b);
}

// Async function
fun fetchCalculationHistory() async -> List<String> {
  let response = await http.get("/api/history");
  return response.json()["history"];
}

fun main() {
  // Basic operations
  print(add(10, 5));        // 15
  print(subtract(10, 5));   // 5
  print(multiply(10, 5));   // 50
  print(divide(10, 5));     // 2.0
  
  // Higher-order function
  print(calculate(10, 5, add));      // 15
  print(calculate(10, 5, multiply)); // 50
  
  // Async operation
  let history = await fetchCalculationHistory();
  print("History: $history");
}
```

## Next Steps

- [Control Flow](04-control-flow.md) - Conditionals and loops
- [Classes](05-classes.md) - Object-oriented programming
- [Error Handling](06-error-handling.md) - Try/catch and Result types

## Need Help?

- [API Reference](../api/functions.md)
- [Examples](https://github.com/zenith-lang/examples)
- [Discord Community](https://discord.gg/zenith)
