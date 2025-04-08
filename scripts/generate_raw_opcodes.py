#!/usr/bin/python3

# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "bs4",
# ]
# ///

from parse_opcode_info import get_opcodes_info, SCRIPT_DIR
import os
import re

def main() -> None:
    with open(
            os.path.join(SCRIPT_DIR, "templates", "raw_opcodes.h.template"),
            encoding="utf-8"
            ) as f:
        text = f.read()

    opcodes = get_opcodes_info()
    PREFIX = "MNEM_"
    mnemonics = [row["name"] for row in opcodes]
    # хочу найти все уникальные мнемоники, но при этом сохранить
    # порядок их объявления. Альтернативы, вроде list(set(mnemonics)).sort()
    # пусть красивы, менее хорошо читаются
    unique_mnemonics = []
    for mnemonic in mnemonics:
        if mnemonic in unique_mnemonics:
            continue
        unique_mnemonics.append(mnemonic)
    formated_mnemonics = list(map(
        lambda s: f"  {PREFIX}{s}",
        unique_mnemonics
        ))
    substitution = '\n'.join(formated_mnemonics)
    with open(
            os.path.join(SCRIPT_DIR, "..", "C6502", "m6502_mnemonics.h"),
            "w",
            encoding="utf-8"
            ) as f:
        f.write(re.sub(r"{{UNIQUE_MNEMONICS}}", substitution, text))

if __name__ == "__main__":
    main()
