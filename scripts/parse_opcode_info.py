# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "bs4",
# ]
# ///

import os
from bs4 import BeautifulSoup, Tag
from typing import TypedDict

SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))

class OpcodeInfo(TypedDict):
    name: str
    opcode: int
    addressing: str
    size: int
    cycles: int
    comment: str

# уровень вложенности здесь некрасивый, но этот скрипт
# писался быстро. Прошу прощения
def _parse_opcode_info(html: str) -> list[OpcodeInfo]:
    """
    Парсит закешированную страницу (может использоваться только для
    определенной страницы в интернете, не универсален) и возвращает
    структурированный словарь с информацией

    :html: страница html в виде простой строки. Может быть получена
           прямым запросом к серверу или обращением к кешированной
           версии в репозитории
    :return: стркутурированный словарь с информацией об опкодах
             из таблицы
    """
    result: list[OpcodeInfo] = list()
    soup = BeautifulSoup(html, "html.parser")
    opcodes = soup.find("dl", class_="opcodes")
    assert isinstance(opcodes, Tag)
    for dt, dd in zip (opcodes.find_all("dt"), opcodes.find_all("dd")):
        name = dt.get_text()
        summary = dd.find("p", {"aria-label": "summary"}).get_text()
        table = dd.find("table", {"aria-label":  "details"})
        rows = table.find_all("tr")
        header_fields = [field.get_text() for field in rows[0].find_all("th")]
        for row in rows[1:]:
            row_fields = {header_fields[name_index]: field.get_text().strip(" \n\t*")
                          for name_index, field in enumerate(row.find_all("td"))}
            result.append(
                    OpcodeInfo(
                        name=name,
                        addressing=row_fields["addressing"],
                        cycles=int(row_fields["cycles"]),
                        opcode=int(row_fields["opc"], 16),
                        size=int(row_fields["bytes"]),
                        comment=summary
                        )
                    )
    return result

def get_opcodes_info() -> list[OpcodeInfo]:
    """
    Обращается к закешированной в репозитории версии страницы
    https://www.masswerk.at/6502/6502_instruction_set.html. Извлекает
    из нее информацию об опкодах и возвращает список
    структурированных словарей
    """
    with open(os.path.join(SCRIPT_DIR, "cached.html")) as f:
        html = f.read()
    return _parse_opcode_info(html)

if __name__ == "__main__":
    print(*get_opcodes_info(), sep='\n')
