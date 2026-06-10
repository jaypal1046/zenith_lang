#!/usr/bin/env python3
"""
Zenith UI Transpiler
Compiles .zen UI blocks into native frameworks:
- Android: Jetpack Compose (Kotlin)
- iOS: SwiftUI
- Web: React + Tailwind
"""

import re
import os
import json
import sys
from pathlib import Path
from typing import Dict, List, Any

class ZenithUITranspiler:
    def __init__(self):
        self.component_map = {
            'Container': {'android': 'Box', 'ios': 'VStack', 'web': 'div'},
            'Text': {'android': 'Text', 'ios': 'Text', 'web': 'span'},
            'Button': {'android': 'Button', 'ios': 'Button', 'web': 'button'},
            'Input': {'android': 'TextField', 'ios': 'TextField', 'web': 'input'},
            'List': {'android': 'LazyColumn', 'ios': 'List', 'web': 'ul'},
            'Image': {'android': 'Image', 'ios': 'Image', 'web': 'img'},
        }

    def parse_zenith_ui(self, source_code: str) -> List[Dict]:
        """Extract UI blocks from Zenith source"""
        ui_blocks = []
        pattern = r'ui\s+(\w+)\s*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}'
        
        for match in re.finditer(pattern, source_code, re.DOTALL):
            name = match.group(1)
            content = match.group(2)
            ui_blocks.append({
                'name': name,
                'content': content,
                'components': self.parse_components(content)
            })
        return ui_blocks

    def parse_components(self, content: str) -> List[Dict]:
        """Parse individual components within a UI block"""
        components = []
        # Simple regex parser for demonstration
        comp_pattern = r'(\w+)\s*\(([^)]*)\)'
        
        for match in re.finditer(comp_pattern, content):
            comp_type = match.group(1)
            args_str = match.group(2)
            
            # Parse arguments
            args = {}
            if args_str.strip():
                arg_pairs = re.findall(r'(\w+):\s*([^,]+)', args_str)
                for key, val in arg_pairs:
                    args[key.strip()] = val.strip().strip('"\'')
            
            components.append({
                'type': comp_type,
                'args': args
            })
        return components

    def generate_compose(self, ui_block: Dict) -> str:
        """Generate Jetpack Compose Kotlin code"""
        lines = [f"@Composable", f"fun {ui_block['name']}() {{"]
        
        for comp in ui_block['components']:
            k_type = self.component_map.get(comp['type'], {}).get('android', comp['type'])
            args = ", ".join([f"{k} = {v}" for k, v in comp['args'].items()])
            lines.append(f"    {k_type}({args})")
            
        lines.append("}")
        return "\n".join(lines)

    def generate_swiftui(self, ui_block: Dict) -> str:
        """Generate SwiftUI code"""
        lines = [f"struct {ui_block['name']}: View {{", "    var body: some View {"]
        
        indent = "        "
        for comp in ui_block['components']:
            i_type = self.component_map.get(comp['type'], {}).get('ios', comp['type'])
            args = " ".join([f".{k}({v})" for k, v in comp['args'].items()])
            lines.append(f"{indent}{i_type}(){args}")
            
        lines.append("    }", "}")
        return "\n".join(lines)

    def generate_react(self, ui_block: Dict) -> str:
        """Generate React + Tailwind code"""
        comp_name = ui_block['name']
        pascal_name = comp_name[0].upper() + comp_name[1:]
        
        lines = [f"export function {pascal_name}() {{", "  return ("]
        
        # Simple flat structure for demo
        items = []
        for comp in ui_block['components']:
            w_type = self.component_map.get(comp['type'], {}).get('web', comp['type'])
            attrs = " ".join([f'{k}={{{v}}}' if not v.startswith('"') else f'{k}={v}' 
                             for k, v in comp['args'].items()])
            items.append(f"    <{w_type} {attrs} />")
            
        if len(items) == 1:
            lines.append(items[0])
        else:
            lines.append("    <div>")
            lines.extend(items)
            lines.append("    </div>")
            
        lines.append("  );", "}")
        return "\n".join(lines)

    def transpile_file(self, input_path: str, output_dir: str):
        """Main entry point to transpile a .zen file"""
        with open(input_path, 'r') as f:
            source = f.read()
            
        ui_blocks = self.parse_zenith_ui(source)
        
        os.makedirs(output_dir, exist_ok=True)
        
        results = {'android': [], 'ios': [], 'web': []}
        
        for block in ui_blocks:
            results['android'].append(self.generate_compose(block))
            results['ios'].append(self.generate_swiftui(block))
            results['web'].append(self.generate_react(block))
            
        # Write outputs
        with open(os.path.join(output_dir, 'MainActivity.kt'), 'w') as f:
            f.write("package com.zenith.app\n\nimport androidx.compose.*\n\n")
            f.write("\n\n".join(results['android']))
            
        with open(os.path.join(output_dir, 'ContentView.swift'), 'w') as f:
            f.write("import SwiftUI\n\n")
            f.write("\n\n".join(results['ios']))
            
        with open(os.path.join(output_dir, 'App.jsx'), 'w') as f:
            f.write("import React from 'react';\n\n")
            f.write("\n\n".join(results['web']))
            
        print(f"✅ Transpiled {len(ui_blocks)} UI blocks to Android, iOS, and Web")
        return results

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: zenith_transpiler.py <input.zen> [output_dir]")
        sys.exit(1)
        
    input_file = sys.argv[1]
    output_folder = sys.argv[2] if len(sys.argv) > 2 else "./build/native_ui"
    
    transpiler = ZenithUITranspiler()
    transpiler.transpile_file(input_file, output_folder)
