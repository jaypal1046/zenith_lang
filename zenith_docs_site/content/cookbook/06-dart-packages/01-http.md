# 🎯 Dart Packages - HTTP Requests

Learn how to use Dart packages in Zenith for making HTTP requests.

## 📋 What You'll Learn

- Using Dart `http` package
- Making GET/POST requests
- Handling JSON responses
- Error handling
- Advanced patterns

## 🎯 Prerequisites

- Zenith SDK 1.0+
- Basic understanding of async/await

## 📝 Step-by-Step Guide

### 1. Add Dependencies

```yaml
# pubspec.yaml
dependencies:
  http: ^1.1.0
  zenith_dart_bridge: ^1.0.0
```

### 2. Generate Bindings

```bash
zenith dart-bindings generate
```

### 3. Make HTTP Requests

```zenith
import 'package:http/http.dart' as http;
import 'dart:convert';

void main() {
  runApp(HttpDemoApp());
}

class HttpDemoApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: Text('🎯 HTTP Requests with Dart')),
        body: HttpExample(),
      ),
    );
  }
}

class HttpExample extends StatefulWidget {
  @override
  _HttpExampleState createState() => _HttpExampleState();
}

class _HttpExampleState extends State<HttpExample> {
  String responseData = '';
  bool isLoading = false;
  
  // GET request
  Future<void> fetchUsers() async {
    setState(() {
      isLoading = true;
      responseData = '';
    });
    
    try {
      final response = await http.get(
        Uri.parse('https://jsonplaceholder.typicode.com/users'),
      );
      
      if (response.statusCode == 200) {
        final List<dynamic> users = json.decode(response.body);
        setState(() {
          responseData = 'Fetched ${users.length} users:\n\n';
          for (var user in users.take(5)) {
            responseData += '- ${user['name']} (${user['email']})\n';
          }
        });
      } else {
        setState(() {
          responseData = 'Error: ${response.statusCode}';
        });
      }
    } catch (e) {
      setState(() {
        responseData = 'Exception: $e';
      });
    } finally {
      setState(() {
        isLoading = false;
      });
    }
  }
  
  // POST request
  Future<void> createUser() async {
    setState(() {
      isLoading = true;
      responseData = '';
    });
    
    try {
      final response = await http.post(
        Uri.parse('https://jsonplaceholder.typicode.com/users'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode({
          'name': 'John Doe',
          'email': 'john@example.com',
          'username': 'johnd',
        }),
      );
      
      if (response.statusCode == 201) {
        final data = json.decode(response.body);
        setState(() {
          responseData = 'Created user with ID: ${data['id']}';
        });
      }
    } catch (e) {
      setState(() {
        responseData = 'Exception: $e';
      });
    } finally {
      setState(() {
        isLoading = false;
      });
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: EdgeInsets.all(20),
      child: Column(
        children: [
          ElevatedButton.icon(
            onPressed: isLoading ? null : fetchUsers,
            icon: Icon(Icons.download),
            label: Text('GET - Fetch Users'),
          ),
          SizedBox(height: 16),
          ElevatedButton.icon(
            onPressed: isLoading ? null : createUser,
            icon: Icon(Icons.upload),
            label: Text('POST - Create User'),
          ),
          SizedBox(height: 24),
          if (isLoading)
            CircularProgressIndicator()
          else if (responseData.isNotEmpty)
            Expanded(
              child: Container(
                width: double.infinity,
                padding: EdgeInsets.all(16),
                decoration: BoxDecoration(
                  color: Colors.grey[100],
                  borderRadius: BorderRadius.circular(8),
                ),
                child: SingleChildScrollView(
                  child: Text(
                    responseData,
                    style: TextStyle(fontFamily: 'monospace'),
                  ),
                ),
              ),
            ),
        ],
      ),
    );
  }
}
```

### 4. Using Dio Package (Advanced)

```zenith
import 'package:dio/dio.dart';

class DioExample extends StatelessWidget {
  final dio = Dio(BaseOptions(
    baseUrl: 'https://api.example.com',
    connectTimeout: Duration(seconds: 5),
    receiveTimeout: Duration(seconds: 3),
  ));
  
  Future<void> fetchData() async {
    try {
      final response = await dio.get('/users/1');
      print('Data: ${response.data}');
    } on DioException catch (e) {
      print('Dio error: ${e.message}');
    }
  }
  
  @override
  Widget build(BuildContext context) {
    return ElevatedButton(
      onPressed: fetchData,
      child: Text('Fetch with Dio'),
    );
  }
}
```

## 💡 Best Practices

1. **Use try-catch** - Always handle network errors
2. **Show loading states** - Provide user feedback
3. **Cancel requests** - Dispose HTTP clients properly
4. **Use interceptors** - For auth tokens and logging
5. **Cache responses** - Reduce unnecessary requests

## 📚 Related Recipes

- [Provider State Management](02-provider.md)
- [Firebase Integration](03-firebase.md)
- [SQLite Database](04-sqlite.md)

---

**Next**: [Provider →](02-provider.md)
