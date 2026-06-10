# Zenith Plugin Manifest Format (.zenplugin)

Defines a type-safe plugin interface for automatic code generation and IDE support.

## Specification

```yaml
# Example: location.zenplugin
name: location
version: 1.0.0
description: GPS location services
platforms: [android, ios, web]

# Dart plugin binding (pub.dev)
dart:
  package: location
  version: ^5.0.0
  class: Location
  
# Rust crate binding (crates.io)  
rust:
  crate: geolocator
  version: "0.1"
  ffi_prefix: geo_

# Type-safe API definitions
functions:
  - name: get_current_position
    description: Get current GPS coordinates
    params:
      - name: accuracy
        type: string
        required: false
        default: "balanced"
        options: ["low", "balanced", "high"]
    returns:
      type: object
      properties:
        latitude: number
        longitude: number
        altitude: number?
        timestamp: number
        
  - name: start_location_updates
    description: Start continuous location streaming
    params:
      - name: interval_ms
        type: number
        required: true
    returns:
      type: stream
      item_type: Position
      
  - name: stop_location_updates
    description: Stop location streaming
    params: []
    returns:
      type: void

# Events emitted by plugin
events:
  - name: on_location_changed
    payload: Position
    
# TypeScript-like type definitions (auto-generated)
types:
  Position:
    latitude: number
    longitude: number
    altitude: number | null
    accuracy: number
    timestamp: number
```

## Auto-Generated Outputs

### 1. TypeScript Definitions (.d.ts)
```typescript
declare module 'native:location' {
  interface Position {
    latitude: number;
    longitude: number;
    altitude: number | null;
    accuracy: number;
    timestamp: number;
  }
  
  export function get_current_position(opts?: {
    accuracy?: 'low' | 'balanced' | 'high';
  }): Promise<Position>;
  
  export function start_location_updates(interval_ms: number): AsyncIterable<Position>;
  export function stop_location_updates(): void;
}
```

### 2. IDE Auto-Complete JSON
```json
{
  "location": {
    "get_current_position": {
      "params": [{"name": "accuracy", "type": "string", "options": ["low", "balanced", "high"]}],
      "returns": "Promise<Position>"
    }
  }
}
```

### 3. Zenith Type Stub (.zen.types)
```zenith
// Auto-generated from .zenplugin
module native:location {
  type Position = {
    latitude: num,
    longitude: num,
    altitude: num?,
    accuracy: num,
    timestamp: num
  };
  
  func get_current_position({accuracy?: "low"|"balanced"|"high"}): Position;
  func start_location_updates(interval_ms: num): Stream<Position>;
  func stop_location_updates(): void;
}
```

## Benefits

✅ **Type Safety** - Compile-time checking of plugin calls
✅ **IDE Support** - Auto-complete, hover docs, go-to-definition
✅ **Documentation** - Auto-generated API docs
✅ **Validation** - Runtime parameter validation
✅ **Cross-Platform** - Single source of truth for all platforms

## Usage in Zenith Code

```zenith
import "native:location" as loc;

// Full type checking and auto-complete
var pos = loc.get_current_position({accuracy: "high"});
print(pos.latitude); // ✅ Valid
print(pos.invalid);  // ❌ Compile error: Property doesn't exist
```

## Tooling Commands

```bash
# Generate type definitions from .zenplugin
zenith plugin gen-types ./plugins/location.zenplugin

# Validate plugin manifest
zenith plugin validate ./plugins/*.zenplugin

# Generate documentation
zenith plugin docs ./plugins --output ./docs/plugins

# Create new plugin template
zenith plugin create my-plugin --dart http --rust reqwest
```
