#!/usr/bin/env python3

# You won't believe how much time this took me

import lldb
import threading
import sys
import re
import os
import time

def parse_image_base_from_log(log_file):
    if not log_file:
        return None

    with open(log_file, 'r', errors='ignore') as f:
        content = f.read()
        pattern = re.compile(r'LoadedImage\-\>ImageBase[:\s]+0x([0-9A-Fa-f]+)+\n', re.IGNORECASE)
        match = pattern.search(content)
        if match:
            return f"0x{match.group(1)}"

    return None

def load_symbols_at_base(debugger, image_base):
    target = debugger.GetSelectedTarget()

    if not target.IsValid():
        print("No valid target")
        return False

    base_addr = int(image_base, 16)
    error = lldb.SBError()
    module = target.AddModule("Snake.debug", None, None)

    if not module.IsValid():
        print(f"Failed to add module")
        return False

    print(f"Loading symbols at ImageBase {image_base}")

    for section in module.section_iter():
        section_name = section.GetName()
        section_addr = section.GetFileAddress()

        if section_addr != lldb.LLDB_INVALID_ADDRESS:
            load_addr = base_addr + section_addr
            target.SetSectionLoadAddress(section, load_addr)

    return True

def wait_and_load_symbols(debugger, log_file, timeout):
    print(f"Waiting for ImageBase in {log_file}")

    start_time = time.time()

    while time.time() - start_time < timeout:
        image_base = parse_image_base_from_log(log_file)
        if image_base:
            print(f"Detected ImageBase: {image_base}")
            return load_symbols_at_base(debugger, image_base)
    return False

def load_uefi_symbols(debugger, command, result, internal_dict):
    args = command.split()
    if len(args) < 1:
        print("Usage: load_uefi_symbols <address>")
        return

    image_base = args[0]
    load_symbols_at_base(debugger, image_base)

def auto_load_symbols(debugger, command, result, internal_dict):
    args = command.split()
    log_file = args[0] if len(args) > 0 else None
    timeout = int(args[1]) if len(args) > 1 else 30

    if not log_file:
        print("Cannot find debug.log")
        print("Usage: auto_load_symbols <log_file> [timeout]")
        return

    print("Running watcher in separate thread...")
    wait_and_load_symbols(debugger, log_file, timeout)

def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
        'command script add -f lldb_uefi_helper.load_uefi_symbols load_uefi_symbols'
    )
    debugger.HandleCommand(
        'command script add -f lldb_uefi_helper.auto_load_symbols auto_load_symbols'
    )
    print("UEFI symbol loader ready")
    print("Commands:")
    print("  load_uefi_symbols <address>              - Manual load")
    print("  auto_load_symbols [log_file] [timeout]   - Auto-detect from log")
