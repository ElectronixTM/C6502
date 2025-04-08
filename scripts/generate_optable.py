# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "bs4",
# ]
# ///

"""
Создает файл имплементации opcodes.c, создавая массив со всеми опкодами
и их характерристиками
"""

from parse_opcode_info import get_opcodes_info, SCRIPT_DIR

OPCODE_TABLE_SIZE = 16*16
NOP_OPCODE = 234
SUBSTITUTIONS = {
        "accumulator": "A",
        "absolute": "ABS",
        "relative": "REL",
        "immediate": "IMM",
        "implied" : "IMPL",
        "indirect": "IND",
        "zeropage": "ZPG",
        "(": "",
        ")": "",
        ",": "_",
        "IND_X": "X_IND" # особенности записи в перечислении
        }
TEMPLATE = "{MNEM_%(name)s, %(size)d, %(cycles)d, %(addressing)s}"

def _reformat_addressing(addressing: str) -> str:
    """
    Переводит то, что указано в описании адрессации из спаршенных данных
    в форму, принятую в в коде проекта
    """
    PREFIX = "AM_"
    result = PREFIX + addressing
    for key, value in SUBSTITUTIONS.items():
        result = result.replace(key, value)
    return result

def main() -> None:
    opcodes = get_opcodes_info()
    # change addressing format
    for info in opcodes:
        info["addressing"] = _reformat_addressing(info["addressing"])
    opcode_to_desc = {info["opcode"]: info for info in opcodes}
    cells: list[str] = []
    for i in range(OPCODE_TABLE_SIZE):
        cell = TEMPLATE % opcode_to_desc[NOP_OPCODE]
        if not i in opcode_to_desc:
            cells.append(cell)
            continue
        opcode_desc = opcode_to_desc[i]
        cell = TEMPLATE % opcode_desc
        cells.append(cell)
    print(*cells, sep='\n')

if __name__=="__main__":
    main()

