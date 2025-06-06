"""
title: Terminal Command Executor
author: Dogturd Stynx
description: evaluate command in executor and return stdout
version: 0.0.1
"""
import os


class Tools:
    def __init__(self):
        pass

    def run_in_terminal(self, command: str) -> str:
        """
        Opens a new terminal window, executes the command, captures output, and closes when done
        :param command: The command to execute
        :return: terminal stdout string
        """
        c = command.split(' ')
        if "rm" in c:
            raise Exception("removing files or directories is not supported")
        else:
            return subprocess.run(o.split(' '), capture_output=True, text=True).stdout
