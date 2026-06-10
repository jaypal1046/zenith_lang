# 🐍 Snake Game - Complete Tutorial

Build a classic Snake game using 100% Zenith code. This tutorial covers game loops, collision detection, and rendering.

## 📋 What You'll Learn

- Game loop implementation
- Grid-based movement
- Collision detection
- Score tracking
- Keyboard input handling

## 🎯 Final Result

A fully functional Snake game that runs on:
- ✅ Web
- ✅ Windows
- ✅ macOS
- ✅ Linux
- ✅ iOS
- ✅ Android

## 📝 Code

```zenith
import 'package:zenith_game/zenith_game.dart';
import 'dart:math';

void main() {
  runApp(SnakeGame());
}

class SnakeGame extends StatefulWidget {
  @override
  _SnakeGameState createState() => _SnakeGameState();
}

class _SnakeGameState extends State<SnakeGame> with TickerProviderStateMixin {
  // Game constants
  static const int gridSize = 20;
  static const int tileCount = 20;
  
  // Game state
  List<Point> snake = [Point(10, 10)];
  Point food = Point(15, 15);
  Point velocity = Point(1, 0);
  int score = 0;
  bool gameOver = false;
  Timer? gameTimer;
  
  // Directions
  final Map<LogicalKeyboardKey, Point> directions = {
    LogicalKeyboardKey.arrowUp: Point(0, -1),
    LogicalKeyboardKey.arrowDown: Point(0, 1),
    LogicalKeyboardKey.arrowLeft: Point(-1, 0),
    LogicalKeyboardKey.arrowRight: Point(1, 0),
  };
  
  @override
  void initState() {
    super.initState();
    startGame();
  }
  
  void startGame() {
    snake = [Point(10, 10)];
    food = spawnFood();
    velocity = Point(1, 0);
    score = 0;
    gameOver = false;
    
    gameTimer = Timer.periodic(Duration(milliseconds: 100), (timer) {
      if (!gameOver) {
        update();
      }
    });
  }
  
  Point spawnFood() {
    final random = Random();
    Point newFood;
    do {
      newFood = Point(
        random.nextInt(tileCount),
        random.nextInt(tileCount),
      );
    } while (snake.contains(newFood));
    return newFood;
  }
  
  void update() {
    // Calculate new head position
    final head = snake.first;
    final newHead = Point(
      (head.x + velocity.x + tileCount) % tileCount,
      (head.y + velocity.y + tileCount) % tileCount,
    );
    
    // Check collision with self
    if (snake.contains(newHead)) {
      gameOver = true;
      gameTimer?.cancel();
      return;
    }
    
    // Move snake
    snake.insert(0, newHead);
    
    // Check if ate food
    if (newHead == food) {
      score += 10;
      food = spawnFood();
    } else {
      snake.removeLast();
    }
    
    setState(() {});
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('🐍 Snake Game - Score: $score'),
        actions: [
          IconButton(
            icon: Icon(Icons.refresh),
            onPressed: startGame,
          ),
        ],
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            if (gameOver)
              Container(
                padding: EdgeInsets.all(20),
                color: Colors.red.withOpacity(0.2),
                child: Column(
                  children: [
                    Text(
                      'Game Over!',
                      style: TextStyle(fontSize: 32, fontWeight: FontWeight.bold),
                    ),
                    SizedBox(height: 10),
                    Text(
                      'Final Score: $score',
                      style: TextStyle(fontSize: 24),
                    ),
                    SizedBox(height: 20),
                    ElevatedButton(
                      onPressed: startGame,
                      child: Text('Play Again'),
                    ),
                  ],
                ),
              ),
            SizedBox(height: 20),
            Container(
              width: gridSize * tileCount.toDouble(),
              height: gridSize * tileCount.toDouble(),
              decoration: BoxDecoration(
                border: Border.all(color: Colors.grey, width: 2),
                color: Colors.black,
              ),
              child: CustomPaint(
                painter: GamePainter(
                  snake: snake,
                  food: food,
                  gridSize: gridSize,
                ),
              ),
            ),
            SizedBox(height: 20),
            Text(
              'Use Arrow Keys to Move',
              style: TextStyle(fontSize: 16, color: Colors.grey),
            ),
          ],
        ),
      ),
      keyboardListener: KeyboardListener(
        onKeyEvent: (KeyEvent event) {
          if (event is KeyDownEvent) {
            final direction = directions[event.logicalKey];
            if (direction != null) {
              // Prevent 180-degree turns
              if (direction.x != -velocity.x || direction.y != -velocity.y) {
                velocity = direction;
              }
            }
          }
        },
      ),
    );
  }
  
  @override
  void dispose() {
    gameTimer?.cancel();
    super.dispose();
  }
}

class GamePainter extends CustomPainter {
  final List<Point> snake;
  final Point food;
  final int gridSize;
  
  GamePainter({
    required this.snake,
    required this.food,
    required this.gridSize,
  });
  
  @override
  void paint(Canvas canvas, Size size) {
    final paint = Paint();
    
    // Draw snake
    paint.color = Colors.green;
    for (final segment in snake) {
      canvas.drawRect(
        Rect.fromLTWH(
          segment.x * gridSize.toDouble(),
          segment.y * gridSize.toDouble(),
          gridSize.toDouble() - 2,
          gridSize.toDouble() - 2,
        ),
        paint,
      );
    }
    
    // Draw food
    paint.color = Colors.red;
    canvas.drawOval(
      Rect.fromLTWH(
        food.x * gridSize.toDouble() + 2,
        food.y * gridSize.toDouble() + 2,
        gridSize.toDouble() - 4,
        gridSize.toDouble() - 4,
      ),
      paint,
    );
  }
  
  @override
  bool shouldRepaint(covariant GamePainter oldDelegate) => true;
}

class Point {
  final int x;
  final int y;
  
  Point(this.x, this.y);
  
  @override
  bool operator ==(Object other) =>
      other is Point && other.x == x && other.y == y;
  
  @override
  int get hashCode => x.hashCode ^ y.hashCode;
}
```

## 🚀 Running the Game

```bash
# Run on web
zenith run web

# Run on desktop
zenith run windows
zenith run macos
zenith run linux

# Run on mobile
zenith run android
zenith run ios
```

## 🎮 Controls

- **Arrow Up**: Move up
- **Arrow Down**: Move down
- **Arrow Left**: Move left
- **Arrow Right**: Move right
- **R / Refresh Button**: Restart game

## 💡 Enhancements to Try

1. Add sound effects
2. Implement high score tracking
3. Add difficulty levels (speed increase)
4. Create obstacles
5. Add multiplayer mode
6. Implement power-ups

## 📚 Related Recipes

- [Pong Game](02-pong.md) - Another classic game
- [Platformer Basics](03-platformer.md) - Jump and run games
- [Physics Engine](04-physics.md) - Add realistic physics

---

**Next**: [Pong Game →](02-pong.md)
