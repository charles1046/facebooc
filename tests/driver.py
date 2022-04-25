#!/usr/bin/python3
import os
import re


def get_all_units() -> list():
    l = []
    for _, _, files in os.walk("./tests/", topdown=False):
        for name in files:
            if(re.search("^.*.o$", name)):
                l.append(name)
    return l


if __name__ == '__main__':
    files = get_all_units()
    print(files)
