import os

# --- DUCKYSCRIPT MAPPER ---
def parse_ducky(line):
    line = line.strip()
    if not line or line.startswith("REM"): return ""
    
    parts = line.split(" ", 1)
    cmd = parts[0].upper()
    args = parts[1] if len(parts) > 1 else ""
    
    # Escape strings for C++
    args_escaped = args.replace('\\', '\\\\').replace('"', '\\"')

    if cmd == "DELAY":     return f"DigiKeyboard.delay({args});"
    if cmd == "STRING":    return f'DigiKeyboard.print("{args_escaped}");'
    if cmd == "ENTER":     return "DigiKeyboard.sendKeyStroke(KEY_ENTER);"
    if cmd == "GUI" or cmd == "WINDOWS":
        if args: return f"DigiKeyboard.sendKeyStroke(KEY_{args.upper()}, MOD_GUI_LEFT);"
        else:    return "DigiKeyboard.sendKeyStroke(0, MOD_GUI_LEFT);"
    if cmd == "MENU":      return "DigiKeyboard.sendKeyStroke(101);"
    if cmd == "APP":       return "DigiKeyboard.sendKeyStroke(101);"
    if cmd == "SHIFT":     return "DigiKeyboard.sendKeyStroke(KEY_" + args.upper() + ", MOD_SHIFT_LEFT);"
    
    return f"// Unknown Ducky: {line}"

# --- MAIN PROCESSOR ---
chambers = {
    "chamber-one": "fire_chamber_one",
    "chamber-two": "fire_chamber_two",
    "chamber-three": "fire_chamber_three"
}

base_path = "src"

for folder, func_name in chambers.items():
    folder_path = os.path.join(base_path, folder)
    if not os.path.exists(folder_path):
        os.makedirs(folder_path)
        
    # Get all .txt and .ino files, sorted alphabetically
    files = sorted([f for f in os.listdir(folder_path) if f.endswith(('.txt', '.ino'))])
    
    cpp_body = []
    
    print(f"Processing {folder}...")
    
    for filename in files:
        filepath = os.path.join(folder_path, filename)
        print(f"  - Merging: {filename}")
        
        with open(filepath, "r") as f:
            lines = f.readlines()
            
        if filename.endswith(".txt"):
            # Convert Ducky
            for line in lines:
                cpp_body.append("  " + parse_ducky(line))
        elif filename.endswith(".ino"):
            # Raw C++ (Strip headers if they exist to avoid conflicts)
            for line in lines:
                if "#include" not in line and "void setup" not in line and "void loop" not in line:
                    cpp_body.append("  " + line.rstrip())

    # Write the payload.h
    header_content = f"""
#ifndef {func_name.upper()}_H
#define {func_name.upper()}_H
#include "DigiKeyboard.h"

void {func_name}() {{
{chr(10).join(cpp_body)}
}}
#endif
    """
    
    with open(os.path.join(folder_path, "payload.h"), "w") as f:
        f.write(header_content)

print("Weaponization Complete.")
