#!/usr/bin/python3

# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "bs4",
# ]
# ///

"""
Скрипт, открывающий файл, парсящий оттуда места для замен и генерирующий
новый файл на основе паттерна, указанного в файле шаблона
"""

import parse_opcode_info
import re
import argparse

OPCODES = parse_opcode_info.get_opcodes_info()

def _substitute_match(template_text: str, match: re.Match) -> str:
    print(match.group(1))
    substitution = ''.join([match.group(1) % opcode for opcode in OPCODES])
    lhs = template_text[:match.start()]
    rhs = template_text[match.end():]
    return lhs + substitution + rhs

def _process_template(template_text: str) -> str:
    """
    Заменяет все вхождения {{"some fmt"}} на нужные по формату строки.
    Поскольку скрипт исключительно утилитарный, пока что это всегда
    ведет к созданию целого списка опкодов по заданному описанию
    """
    for match in re.finditer(r'{{"(.*)"}}', template_text):
        template_text = _substitute_match(template_text, match)
    return template_text

def _remove_comments(text: str) -> str:
    return re.sub(r"\/\*TEMPLATE COMMENT.*\*\/", "", text, flags=re.DOTALL)

def main() -> None:
    argparser = argparse.ArgumentParser(
            prog="templater",
            description="Simple program to generate C files with "
                        "embeding opcodes info inside"
            )
    argparser.add_argument(
            "template",
            help="template file to generate C source from"
            )
    argparser.add_argument(
            "output",
            default="",
            help="file to store result substitution to"
            )
    args = argparser.parse_args()
    with open(args.template, 'r', encoding="utf-8") as f:
        text = f.read()
    try:
        text = _remove_comments(text)
        text = _process_template(text)
    except:
        print("Error accured while processing template. "
              "Check your template file")
        return
    with open(args.output, 'w', encoding="utf-8") as f:
        f.write(text)

if __name__ == "__main__":
    main()
