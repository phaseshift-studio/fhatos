import json
import configparser
import pprint
from os import listdir
from os.path import isfile, join

boards = dict()
config = configparser.ConfigParser()
config.read('../../../platformio.ini')
sections = filter(lambda x: x.startswith('env:'), config.sections())
for s in sections:
    key = config[s].name.removeprefix("env:")
    file = config[s].get("board")
    for f in listdir("../../../board/"):
        if (f.endswith(".json") and f.startswith(file)):
            data = json.load(open(f"../../../board/{f}", "r"))
            value = dict()
            value["name"] = data["name"].lower()
            value["vendor"] = data["vendor"].lower()
            value["url"] = data["url"]
            boards[key] = value
pprint.pprint(boards)
