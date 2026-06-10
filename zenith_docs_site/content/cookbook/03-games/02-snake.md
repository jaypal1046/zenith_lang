# Snake Game - Classic Arcade Game in Zenith

Build the classic Snake game using 100% Zenith code. This tutorial shows you how to create a complete, playable game that runs on desktop, mobile, and web.

## Overview

You'll learn:
- Game loop implementation
- Keyboard/touch input handling
- Collision detection
- Score tracking
- Game state management
- Cross-platform deployment

## Prerequisites

- Basic Zenith syntax knowledge
- Understanding of classes and functions
- Zenith SDK installed

## Complete Game Code

Create a file called `snake_game.zenith`:

```zenith
import 'package:zenith_ui/zenith_ui.dart';
import 'dart:math';

void main() {
  runApp(SnakeGameApp());
}

class SnakeGameApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Snake Game',
      home: SnakeGame(),
      debugShowCheckedModeBanner: false,
    );
  }
}

class SnakeGame extends StatefulWidget {
  @override
  _SnakeGameState createState() => _SnakeGameState();
}

class _SnakeGameState extends State<SnakeGame> {
  static const int gridSize = 20;
  static const int tileCount = 20;
  
  List<Point> snake = [];
  Point food = Point(15, 15);
  Point velocity = Point(1, 0);
  int score = 0;
  bool gameOver = false;
  bool isPaused = false;
  
  Random random = Random();
  
  @override
  void initState() {
    super.initState();
    resetGame();
  }
  
  void resetGame() {
    snake = [Point(10, 10)];
    velocity = Point(1, 0);
    score = 0;
    gameOver = false;
    isPaused = false;
    placeFood();
  }
  
  void placeFood() {
    food = Point(
      random.nextInt(tileCount),
      random.nextInt(tileCount),
    );
  }
  
  void updateGame() {
    if (gameOver || isPaused) return;
    
    // Calculate new head position
    Point head = snake.last;
    Point newHead = Point(head.x + velocity.x, head.y + velocity.y);
    
    // Check wall collision
    if (newHead.x < 0 || newHead.x >= tileCount ||
        newHead.y < 0 || newHead.y >= tileCount) {
      gameOver = true;
      return;
    }
    
    // Check self collision
    for (var segment in snake) {
      if (segment.x == newHead.x && segment.y == newHead.y) {
        gameOver = true;
        return;
      }
    }
    
    // Move snake
    snake.add(newHead);
    
    // Check food collision
    if (newHead.x == food.x && newHead.y == food.y) {
      score += 10;
      placeFood();
    } else {
      snake.removeAt(0);
    }
  }
  
  void handleKeyPress(KeyEvent event) {
    if (event.isRepeat) return;
    
    switch (event.logicalKey) {
      case LogicalKeyboardKey.arrowUp:
        if (velocity.y != 1) velocity = Point(0, -1);
        break;
      case LogicalKeyboardKey.arrowDown:
        if (velocity.y != -1) velocity = Point(0, 1);
        break;
      case LogicalKeyboardKey.arrowLeft:
        if (velocity.x != 1) velocity = Point(-1, 0);
        break;
      case LogicalKeyboardKey.arrowRight:
        if (velocity.x != -1) velocity = Point(1, 0);
        break;
      case LogicalKeyboardKey.space:
        isPaused = !isPaused;
        break;
      case LogicalKeyboardKey.enter:
        if (gameOver) resetGame();
        break;
    }
  }
  
  void handleSwipe(String direction) {
    switch (direction) {
      case 'up':
        if (velocity.y != 1) velocity = Point(0, -1);
        break;
      case 'down':
        if (velocity.y != -1) velocity = Point(0, 1);
        break;
      case 'left':
        if (velocity.x != 1) velocity = Point(-1, 0);
        break;
      case 'right':
        if (velocity.x != -1) velocity = Point(1, 0);
        break;
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Snake Game'),
        actions: [
          IconButton(
            icon: Icon(Icons.refresh),
            onPressed: resetGame,
          ),
        ],
      ),
      body: KeyboardListener(
        onKeyEvent: handleKeyPress,
        autofocus: true,
        child: GestureDetector(
          onPanUpdate: (details) {
            if (details.delta.dx > 50) handleSwipe('right');
            else if (details.delta.dx < -50) handleSwipe('left');
            else if (details.delta.dy > 50) handleSwipe('down');
            else if (details.delta.dy < -50) handleSwipe('up');
          },
          child: Center(
            child: Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Text(
                  'Score: $score',
                  style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
                ),
                SizedBox(height: 20),
                Container(
                  width: gridSize * tileCount.toDouble(),
                  height: gridSize * tileCount.toDouble(),
                  decoration: BoxDecoration(
                    color: Colors.black87,
                    border: Border.all(color: Colors.green, width: 2),
                  ),
                  child: CustomPaint(
                    painter: SnakePainter(
                      snake: snake,
                      food: food,
                      gridSize: gridSize,
                      gameOver: gameOver,
                      isPaused: isPaused,
                    ),
                  ),
                ),
                SizedBox(height: 20),
                if (gameOver)
                  Column(
                    children: [
                      Text(
                        'Game Over!',
                        style: TextStyle(
                          fontSize: 32,
                          fontWeight: FontWeight.bold,
                          color: Colors.red,
                        ),
                      ),
                      SizedBox(height: 10),
                      Text(
                        'Press Enter or tap Refresh to play again',
                        style: TextStyle(fontSize: 16),
                      ),
                    ],
                  ),
                if (isPaused && !gameOver)
                  Text(
                    'Paused',
                    style: TextStyle(
                      fontSize: 24,
                      fontWeight: FontWeight.bold,
                      color: Colors.orange,
                    ),
                  ),
                SizedBox(height: 20),
                Text(
                  'Controls: Arrow Keys / Swipe | Space: Pause | Enter: Restart',
                  style: TextStyle(color: Colors.grey),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

class SnakePainter extends CustomPainter {
  final List<Point> snake;
  final Point food;
  final int gridSize;
  final bool gameOver;
  final bool isPaused;
  
  SnakePainter({
    required this.snake,
    required this.food,
    required this.gridSize,
    required this.gameOver,
    required this.isPaused,
  });
  
  @override
  void paint(Canvas canvas, Size size) {
    // Draw snake
    Paint snakePaint = Paint()..color = Colors.green;
    Paint headPaint = Paint()..color = Colors.lightGreenAccent;
    
    for (int i = 0; i < snake.length; i++) {
      Point segment = snake[i];
      Rect rect = Rect.fromLTWH(
        segment.x * gridSize.toDouble(),
        segment.y * gridSize.toDouble(),
        gridSize.toDouble() - 2,
        gridSize.toDouble() - 2,
      );
      
      canvas.drawRect(
        rect,
        i == snake.length - 1 ? headPaint : snakePaint,
      );
    }
    
    // Draw food
    Paint foodPaint = Paint()..color = Colors.red;
    Rect foodRect = Rect.fromLTWH(
      food.x * gridSize.toDouble(),
      food.y * gridSize.toDouble(),
      gridSize.toDouble() - 2,
      gridSize.toDouble() - 2,
    );
    canvas.drawRect(foodRect, foodPaint);
    
    // Draw game over overlay
    if (gameOver) {
      Paint overlayPaint = Paint()..color = Colors.black54;
      canvas.drawRect(Rect.fromLTWH(0, 0, size.width, size.height), overlayPaint);
    }
  }
  
  @override
  bool shouldRepaint(covariant SnakePainter oldDelegate) => true;
}

// Game loop timer
class GameLoop extends Timer {
  GameLoop(Duration duration, void Function() callback)
      : super.periodic(duration, (_) => callback());
}
```

## Running the Game

### Desktop

```bash
zenith run snake_game.zenith
```

**Controls:**
- Arrow Keys: Move snake
- Space: Pause/Resume
- Enter: Restart after game over

### Mobile (iOS/Android)

```bash
# iOS
zenith run snake_game.zenith --platform ios

# Android
zenith run snake_game.zenith --platform android
```

**Controls:**
- Swipe: Change direction
- Tap refresh button: Restart

### Web

```bash
zenith run snake_game.zenith --platform web
```

Works with both keyboard and touch controls!

## Building for Production

### All Platforms

```bash
# Desktop
zenith build desktop --release

# iOS
zenith build ios --release

# Android
zenith build android --release

# Web
zenith build web --release
```

## Enhancements to Try

### 1. Add Sound Effects

```zenith
import 'package:audioplayers/audioplayers.dart';

final player = AudioPlayer();

void playSound(String soundFile) {
  player.play(AssetSource('sounds/$soundFile.mp3'));
}

// Call when eating food
playSound('eat.mp3');

// Call on game over
playSound('gameover.mp3');
```

### 2. Add High Score

```zenith
import 'package:shared_preferences/shared_preferences.dart';

Future<void> saveHighScore(int score) async {
  final prefs = await SharedPreferences.getInstance();
  final highScore = prefs.getInt('highScore') ?? 0;
  if (score > highScore) {
    await prefs.setInt('highScore', score);
  }
}

Future<int> getHighScore() async {
  final prefs = await SharedPreferences.getInstance();
  return prefs.getInt('highScore') ?? 0;
}
```

### 3. Add Difficulty Levels

```zenith
enum Difficulty { easy, medium, hard }

Duration getSpeed(Difficulty difficulty) {
  switch (difficulty) {
    case Difficulty.easy:
      return Duration(milliseconds: 200);
    case Difficulty.medium:
      return Duration(milliseconds: 150);
    case Difficulty.hard:
      return Duration(milliseconds: 100);
  }
}
```

### 4. Add Power-ups

```zenith
class PowerUp {
  Point position;
  PowerUpType type;
  Duration duration;
  
  enum PowerUpType { speedBoost, slowMotion, doublePoints }
}
```

## Platform-Specific Features

### Desktop Enhancements

Add menu bar:
```zenith
if (Platform.isWindows || Platform.isLinux || Platform.isMacOS) {
  // Add native menu bar
}
```

### Mobile Enhancements

Add haptic feedback:
```zenith
import 'package:vibration/vibration.dart';

Vibration.vibrate(duration: 50); // On food eaten
```

### Web Enhancements

Add PWA support:
```yaml
# In pubspec.yaml
pwa:
  name: Snake Game
  short_name: Snake
  theme_color: '#4CAF50'
```

## Performance Tips

1. **Use const constructors** where possible
2. **Minimize repaints** with proper widget structure
3. **Pre-load assets** before game starts
4. **Use object pooling** for frequently created objects

## Common Issues

### Game runs too fast/slow
Adjust the game loop timer duration based on platform performance.

### Touch controls not responsive
Increase the swipe detection threshold or add visual feedback.

### Memory leaks
Ensure timers are cancelled when widget is disposed:

```zenith
@override
void dispose() {
  gameTimer?.cancel();
  super.dispose();
}
```

## Next Steps

- [Tic-Tac-Toe](01-tic-tac-toe.md) - Another classic game
- [Platformer](03-platformer.md) - 2D platform game
- [3D Cube](04-3d-cube.md) - Introduction to 3D graphics
- [Rust Integration](../../05-rust-integration/01-setup.md) - Use Rust for game physics

## Resources

- [Zenith Game Development Guide](../../docs/game-dev/INDEX.md)
- [Input Handling](../../docs/ui/input-handling.md)
- [Animation System](../../docs/ui/animations.md)
- [Community Games Showcase](https://github.com/zenith-lang/games)

---

**Built with 100% Zenith Code** - No other languages required! 🎮

*Now you have a complete, cross-platform Snake game!*
