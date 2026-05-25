class Vector2D {
  final double x;
  final double y;
  
  Vector2D(this.x, this.y);
  
  double magnitude() => x * x + y * y;
  
  Vector2D add(Vector2D other) {
    return Vector2D(x + other.x, y + other.y);
  }
}

void printVector(Vector2D v) {
  print("Vector: (${v.x}, ${v.y})");
}
