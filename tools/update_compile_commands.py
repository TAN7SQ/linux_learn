#!/usr/bin/env python3
import json
import os
import shlex
import subprocess
from pathlib import Path


WORKSPACE = Path("/home/tans/workspace/linux_drivers")
KERNEL_SRC = Path("/home/tans/workspace/orangepi-build/kernel/orange-pi-5.10-rk35xx")
KERNEL_BUILD = Path("/home/tans/workspace/kernel_build/rk3566_cm4")
GEN_SCRIPT = KERNEL_SRC / "scripts/clang-tools/gen_compile_commands.py"
OUTPUT = WORKSPACE / "compile_commands.json"
GCC_WRAPPER_NAME = "gcc-wrapper.py"


def built_objects():
    return sorted(
        path
        for path in WORKSPACE.glob("*/build/*.o")
        if not path.name.endswith(".mod.o")
    )


def add_real_source_entries(entries):
    result = []
    seen = set()
    for entry in entries:
        key = (entry.get("directory"), entry.get("file"), entry.get("command"))
        if key not in seen:
            result.append(entry)
            seen.add(key)

        file_path = Path(entry["file"])
        if not file_path.is_symlink():
            continue

        real_file = file_path.resolve()
        if real_file == file_path:
            continue

        real_entry = dict(entry)
        real_entry["file"] = str(real_file)
        real_entry["command"] = entry["command"].replace(str(file_path), str(real_file))
        key = (real_entry.get("directory"), real_entry.get("file"), real_entry.get("command"))
        if key not in seen:
            result.append(real_entry)
            seen.add(key)
    return result


def normalize_command(command):
    parts = shlex.split(command)
    if parts and parts[0].endswith("/" + GCC_WRAPPER_NAME):
        parts = parts[1:]
    return shlex.join(parts)


def normalize_entries(entries):
    normalized = []
    for entry in entries:
        entry = dict(entry)
        entry["command"] = normalize_command(entry["command"])
        normalized.append(entry)
    return normalized


def main():
    objects = built_objects()
    if not objects:
        raise SystemExit("No built module objects found. Run make in a module directory first.")

    subprocess.run(
        [
            "python3",
            str(GEN_SCRIPT),
            "-d",
            str(KERNEL_BUILD),
            "-o",
            str(OUTPUT),
            *map(str, objects),
        ],
        cwd=WORKSPACE,
        check=True,
    )

    entries = normalize_entries(json.loads(OUTPUT.read_text()))
    entries = add_real_source_entries(entries)
    OUTPUT.write_text(json.dumps(entries, indent=2) + "\n")
    print(f"Wrote {OUTPUT} with {len(entries)} entries")


if __name__ == "__main__":
    main()
