# Zenith Feature Audit: Agentic AI Primitives

## 1. Executive Overview
Zenith introduces native programming language primitives for AI agents. Keywords like `agent`, `prompt`, and `tool` are first-class constructs tokenized by the lexer, parsed into AST node representations, and compiled with HTTP client connector bindings.

---

## 2. ✅ Developed Capabilities (Mapped to Source Files)
- **First-Class AI Keywords**:
  - `agent`, `prompt`, `tool` keyword lexing ([lexer.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/lexer.cpp#L42-L48)).
  - AST Parsing & Node construction ([parser.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/parser.cpp), [ast.h](file:///c:/Jay/_Plugin/zenith_lang/include/ast/ast.h)).
- **HTTP / LLM API Connector**:
  - WinHTTP connector for local LLM & OpenAI HTTP API endpoints ([zenith_windows.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/desktop/windows/zenith_windows.h)).
- **Native Tool Binding**:
  - Exposing normal Zenith functions as tools accessible to agents with JSON schema serialization.

---

## 3. ❌ Missing / Incomplete Features
- **Tool Sandbox & Fine-Grained Permissions**: Restricting tool execution capabilities based on security policies.
- **Multi-Agent Inter-Process Message Bus**: Agent-to-agent negotiation protocols.

---

## 4. 💻 How to Use
```zenith
tool fetch_system_logs(lines: Int) -> String {
    return fs.read_string("logs/app.log");
}

agent LogAnalyzer {
    model: "gpt-4o-mini",
    temperature: 0.2,
    tools: [fetch_system_logs]
}

prompt AnalyzeErrors(app_name: String) {
    """
    You are an automated diagnostic agent.
    Analyze system logs for ${app_name} and identify critical exceptions.
    Use fetch_system_logs tool if necessary.
    """
}

async fn run_diagnosis() {
    let analyzer = LogAnalyzer();
    let result = await analyzer.execute(AnalyzeErrors("NebulaEngine"));
    println("Diagnosis Result: " + result);
}
```

---

## 5. ⚙️ Control & Source File Map
| Component | Header File | Implementation File / Source |
| :--- | :--- | :--- |
| **Agent Keyword Lexing** | [lexer.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/lexer.h) | [lexer.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/lexer.cpp) |
| **Agent/Tool Parser & AST** | [parser.h](file:///c:/Jay/_Plugin/zenith_lang/include/frontend/parser.h), [ast.h](file:///c:/Jay/_Plugin/zenith_lang/include/ast/ast.h) | [parser.cpp](file:///c:/Jay/_Plugin/zenith_lang/src/frontend/parser.cpp) |
| **LLM Network Transport** | [zenith_windows.h](file:///c:/Jay/_Plugin/zenith_lang/include/zenith/desktop/windows/zenith_windows.h) | N/A |
