#!/usr/bin/env python3
"""
Zenith DAP (Debug Adapter Protocol) Validation Test Suite

This script validates that the DAP server correctly translates debugger actions
between backend runtime engines (GDB/LLDB) and client IDEs.

Tests include:
- Breakpoint resolution and verification
- Step-over, step-in, step-out operations
- Continue execution
- Stack trace retrieval
- Variable evaluation
- Thread management
"""

import json
import socket
import subprocess
import time
import sys
import os
from typing import Dict, List, Optional, Any

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.stderr.reconfigure(encoding="utf-8", errors="replace")


class DAPTestClient:
    """Test client for validating DAP server functionality."""
    
    def __init__(self, port: int = 4711):
        self.port = port
        self.sequence = 0
        self.sock: Optional[socket.socket] = None
        self.received_events: List[Dict] = []
        
    def connect(self, timeout: float = 5.0) -> bool:
        """Connect to the DAP server."""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(timeout)
            self.sock.connect(('localhost', self.port))
            return True
        except (socket.error, ConnectionRefusedError) as e:
            print(f"Failed to connect to DAP server: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from the DAP server."""
        if self.sock:
            try:
                self.sock.close()
            except:
                pass
            self.sock = None
    
    def _send_request(self, command: str, arguments: Optional[Dict] = None) -> Dict:
        """Send a DAP request and return the response."""
        if not self.sock:
            raise RuntimeError("Not connected to DAP server")
        
        self.sequence += 1
        request = {
            "seq": self.sequence,
            "type": "request",
            "command": command
        }
        if arguments:
            request["arguments"] = arguments
        
        content = json.dumps(request)
        header = f"Content-Length: {len(content)}\r\n\r\n"
        self.sock.sendall((header + content).encode('utf-8'))
        
        # Read response
        return self._receive_response()
    
    def _receive_response(self, timeout: float = 5.0) -> Dict:
        """Receive a DAP response or event."""
        if not self.sock:
            raise RuntimeError("Not connected to DAP server")
        
        self.sock.settimeout(timeout)
        header = b""
        while b"\r\n\r\n" not in header:
            try:
                chunk = self.sock.recv(1)
                if not chunk:
                    raise RuntimeError("Connection closed")
                header += chunk
            except socket.timeout:
                raise RuntimeError("Timeout waiting for header")
        
        header_str = header.decode('utf-8')
        content_length = 0
        for line in header_str.split('\r\n'):
            if line.startswith('Content-Length:'):
                content_length = int(line.split(':')[1].strip())
        
        content = b""
        while len(content) < content_length:
            try:
                chunk = self.sock.recv(content_length - len(content))
                if not chunk:
                    raise RuntimeError("Connection closed")
                content += chunk
            except socket.timeout:
                raise RuntimeError("Timeout waiting for content")
        
        response = json.loads(content.decode('utf-8'))
        
        # Store events separately
        if response.get("type") == "event":
            self.received_events.append(response)
            # Recursively get next response for events
            return self._receive_response(timeout)
        
        return response
    
    def initialize(self) -> bool:
        """Test initialize request."""
        print("  Testing initialize...")
        response = self._send_request("initialize", {
            "clientID": "test-client",
            "adapterID": "zenith-dap",
            "pathFormat": "path",
            "linesStartAt1": True,
            "columnsStartAt1": True
        })
        
        success = response.get("success", False)
        if success:
            capabilities = response.get("body", {})
            print(f"    ✓ Initialize successful")
            print(f"    Capabilities: supportsConfigurationDone={capabilities.get('supportsConfigurationDoneRequest')}, "
                  f"supportsBreakpoints={capabilities.get('supportsFunctionBreakpoints')}")
        else:
            print(f"    ✗ Initialize failed: {response.get('message', 'Unknown error')}")
        
        return success
    
    def set_breakpoints(self, source_path: str, lines: List[int]) -> bool:
        """Test setBreakpoints request."""
        print(f"  Testing setBreakpoints at {source_path}: lines {lines}...")
        breakpoints = [{"line": line} for line in lines]
        
        response = self._send_request("setBreakpoints", {
            "source": {"path": source_path},
            "breakpoints": breakpoints
        })
        
        success = response.get("success", False)
        if success:
            returned_bps = response.get("body", {}).get("breakpoints", [])
            verified_count = sum(1 for bp in returned_bps if bp.get("verified", False))
            print(f"    ✓ Set {len(returned_bps)} breakpoints, {verified_count} verified")
        else:
            print(f"    ✗ SetBreakpoints failed: {response.get('message', 'Unknown error')}")
        
        return success
    
    def launch(self, program: str, args: Optional[List[str]] = None) -> bool:
        """Test launch request."""
        print(f"  Testing launch: {program}...")
        arguments = {"program": program}
        if args:
            arguments["args"] = args
        
        response = self._send_request("launch", arguments)
        
        success = response.get("success", False)
        if success:
            print(f"    ✓ Launch successful")
        else:
            print(f"    ✗ Launch failed: {response.get('message', 'Unknown error')}")
        
        return success
    
    def configuration_done(self) -> bool:
        """Test configurationDone request."""
        print("  Testing configurationDone...")
        response = self._send_request("configurationDone")
        
        success = response.get("success", False)
        if success:
            print(f"    ✓ ConfigurationDone successful")
        else:
            print(f"    ✗ ConfigurationDone failed")
        
        return success
    
    def continue_execution(self) -> bool:
        """Test continue request."""
        print("  Testing continue...")
        response = self._send_request("continue", {"threadId": 1})
        
        success = response.get("success", False)
        if success:
            all_continued = response.get("body", {}).get("allThreadsContinued", False)
            print(f"    ✓ Continue successful (allThreadsContinued={all_continued})")
        else:
            print(f"    ✗ Continue failed")
        
        return success
    
    def next_step(self) -> bool:
        """Test next (step-over) request."""
        print("  Testing next (step-over)...")
        response = self._send_request("next", {"threadId": 1})
        
        success = response.get("success", False)
        if success:
            print(f"    ✓ Next successful")
        else:
            print(f"    ✗ Next failed")
        
        return success
    
    def step_in(self) -> bool:
        """Test stepIn request."""
        print("  Testing stepIn...")
        response = self._send_request("stepIn", {"threadId": 1})
        
        success = response.get("success", False)
        if success:
            print(f"    ✓ StepIn successful")
        else:
            print(f"    ✗ StepIn failed")
        
        return success
    
    def step_out(self) -> bool:
        """Test stepOut request."""
        print("  Testing stepOut...")
        response = self._send_request("stepOut", {"threadId": 1})
        
        success = response.get("success", False)
        if success:
            print(f"    ✓ StepOut successful")
        else:
            print(f"    ✗ StepOut failed")
        
        return success
    
    def threads(self) -> bool:
        """Test threads request."""
        print("  Testing threads...")
        response = self._send_request("threads")
        
        success = response.get("success", False)
        if success:
            thread_list = response.get("body", {}).get("threads", [])
            print(f"    ✓ Threads successful: {len(thread_list)} thread(s)")
            for thread in thread_list:
                print(f"      - Thread {thread.get('id')}: {thread.get('name')}")
        else:
            print(f"    ✗ Threads failed")
        
        return success
    
    def stack_trace(self) -> bool:
        """Test stackTrace request."""
        print("  Testing stackTrace...")
        response = self._send_request("stackTrace", {"threadId": 1})
        
        success = response.get("success", False)
        if success:
            frames = response.get("body", {}).get("stackFrames", [])
            total = response.get("body", {}).get("totalFrames", 0)
            print(f"    ✓ StackTrace successful: {len(frames)} frames (total: {total})")
            for frame in frames[:3]:  # Show first 3 frames
                print(f"      - Frame {frame.get('id')}: {frame.get('name')} at line {frame.get('line')}")
        else:
            print(f"    ✗ StackTrace failed")
        
        return success
    
    def evaluate(self, expression: str) -> bool:
        """Test evaluate request."""
        print(f"  Testing evaluate: '{expression}'...")
        response = self._send_request("evaluate", {
            "expression": expression,
            "frameId": 1
        })
        
        success = response.get("success", False)
        if success:
            result = response.get("body", {}).get("result", "")
            print(f"    ✓ Evaluate successful: {expression} = {result}")
        else:
            print(f"    ✗ Evaluate failed: {response.get('message', 'Unknown error')}")
        
        return success
    
    def disconnect_session(self) -> bool:
        """Test disconnect request."""
        print("  Testing disconnect...")
        response = self._send_request("disconnect", {"restart": False})
        
        success = response.get("success", False)
        if success:
            print(f"    ✓ Disconnect successful")
        else:
            print(f"    ✗ Disconnect failed")
        
        return success
    
    def pause(self) -> bool:
        """Test pause request."""
        print("  Testing pause...")
        response = self._send_request("pause", {"threadId": 1})
        
        success = response.get("success", False)
        if success:
            print(f"    ✓ Pause successful")
        else:
            print(f"    ✗ Pause failed")
        
        return success


def run_dap_validation_tests(dap_server_path: str) -> bool:
    """Run comprehensive DAP validation tests."""
    print("\n" + "="*70)
    print("           ZENITH DAP VALIDATION TEST SUITE")
    print("="*70)
    
    # Start DAP server
    print("\n[Setup] Starting DAP server...")
    dap_process = None
    try:
        dap_process = subprocess.Popen(
            [sys.executable, dap_server_path, "--port", "4711"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=os.path.dirname(dap_server_path)
        )
        time.sleep(2)  # Wait for server to start
        
        # Check if server started successfully
        if dap_process.poll() is not None:
            stdout, stderr = dap_process.communicate()
            print(f"✗ DAP server failed to start:")
            print(f"  stdout: {stdout.decode()}")
            print(f"  stderr: {stderr.decode()}")
            return False
        
        print("  ✓ DAP server started on port 4711")
    except Exception as e:
        print(f"✗ Failed to start DAP server: {e}")
        return False
    
    # Run tests
    client = DAPTestClient(port=4711)
    test_results = []
    
    try:
        if not client.connect():
            print("\n✗ Failed to connect to DAP server")
            return False
        
        print("\n[Test Suite 1] Basic Protocol Tests")
        test_results.append(("Initialize", client.initialize()))
        
        print("\n[Test Suite 2] Breakpoint Tests")
        test_results.append(("SetBreakpoints", client.set_breakpoints("main.zen", [10, 20, 30])))
        
        print("\n[Test Suite 3] Launch & Configuration Tests")
        # Note: These will fail without a real compiled binary, but test protocol translation
        test_results.append(("Launch", client.launch("test_program.exe")))
        test_results.append(("ConfigurationDone", client.configuration_done()))
        
        print("\n[Test Suite 4] Execution Control Tests")
        test_results.append(("Continue", client.continue_execution()))
        test_results.append(("Next", client.next_step()))
        test_results.append(("StepIn", client.step_in()))
        test_results.append(("StepOut", client.step_out()))
        
        print("\n[Test Suite 5] Inspection Tests")
        test_results.append(("Threads", client.threads()))
        test_results.append(("StackTrace", client.stack_trace()))
        test_results.append(("Evaluate", client.evaluate("x")))
        
        print("\n[Test Suite 6] Session Management Tests")
        test_results.append(("Pause", client.pause()))
        test_results.append(("Disconnect", client.disconnect_session()))
        
    finally:
        client.disconnect()
        if dap_process:
            dap_process.terminate()
            try:
                dap_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                dap_process.kill()
    
    # Print summary
    print("\n" + "="*70)
    print("                    TEST RESULTS SUMMARY")
    print("="*70)
    
    passed = sum(1 for _, result in test_results if result)
    failed = sum(1 for _, result in test_results if not result)
    
    for test_name, result in test_results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"  {status}: {test_name}")
    
    print("-"*70)
    print(f"  Total: {passed} Passed / {failed} Failed / {len(test_results)} Tests")
    print("="*70)
    
    # Additional validation: Check event handling
    print("\n[Validation] Event Handling Check")
    if client.received_events:
        print(f"  ✓ Received {len(client.received_events)} event(s) during session")
        for event in client.received_events:
            print(f"    - Event: {event.get('event')}")
    else:
        print("  ℹ No events received (may be expected depending on test scenario)")
    
    return failed == 0


def main():
    """Main entry point for DAP validation tests."""
    import argparse
    
    parser = argparse.ArgumentParser(description="Zenith DAP Validation Test Suite")
    parser.add_argument(
        "--dap-server",
        default=None,
        help="Path to DAP server script (default: auto-detect)"
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable verbose output"
    )
    
    args = parser.parse_args()
    
    # Resolve path
    if args.dap_server:
        dap_server_path = args.dap_server
        if not os.path.isabs(dap_server_path):
            dap_server_path = os.path.join(os.getcwd(), dap_server_path)
    else:
        # Auto-detect: try relative to script directory first
        script_dir = os.path.dirname(os.path.abspath(__file__))
        root_dir = os.path.dirname(script_dir)
        dap_server_path = os.path.join(root_dir, "tools", "dap-server", "zenith_dap_server.py")
    
    if not os.path.exists(dap_server_path):
        print(f"Error: DAP server not found at {dap_server_path}")
        sys.exit(1)
    
    success = run_dap_validation_tests(dap_server_path)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
