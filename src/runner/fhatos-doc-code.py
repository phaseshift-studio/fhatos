# Copyright (c) 2023, Bas Nijholt
# edited by Dogturd Stynx
# All rights reserved.
"""
mm-ADT code runner for FhatOS documentation
"""
from __future__ import annotations

import argparse
import os
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import TYPE_CHECKING, Any

from colors import *

if TYPE_CHECKING:
    try:
        from typing import Literal  # type: ignore[attr-defined]
    except ImportError:
        from typing_extensions import Literal

if sys.version_info >= (3, 8):  # pragma: no cover
    from importlib.metadata import PackageNotFoundError, version

    try:
        __version__ = version("markdown-code-runner")
    except PackageNotFoundError:
        __version__ = "unknown"
else:  # pragma: no cover

    __version__ = pkg_resources.get_distribution("markdown-code-runner").version

DEBUG: bool = os.environ.get("DEBUG", "0") == "1"


def remove_md_comment(commented_text: str) -> str:
    """Remove Markdown comment tags from a string."""
    # commented_text = commented_text.strip()
    commented_text = commented_text.removesuffix(" -->")
    return commented_text.replace("<!-- ", "")
    # if not (commented_text.startswith("<!-- ") and commented_text.endswith(" -->")):
    #    msg = f"Invalid Markdown comment format: {commented_text}"
    #    raise ValueError(msg)
    # return commented_text[5:-4]


def execute_code(
        code: list[str],
        context: dict[str, Any] | None = None,
        language: Literal["python", "bash"] = None,  # type: ignore[name-defined]
        *,
        output_file: str | Path | None = None,
        verbose: bool = False,
) -> list[str]:
    """Execute a code block and return its output as a list of strings."""
    if context is None:
        context = {}
    new_code = []
    cat = False
    for c in code:
        if not new_code:
            new_code.append(c)
        else:
            if cat:
                new_code.append(new_code.pop().rstrip().removesuffix(" /\"") + "\n        " + c + "\"")
            else:
                new_code.append("\"" + c + "\"")
        cat = c.endswith("/")
    full_code = " ".join(new_code)
    full_code = full_code.replace("\\|", "|")

    if verbose:
        print(f"\n🐖 {BOLD}{GREEN}executing code {language} block:{NC}")
        print(f"\n{CYAN}{full_code}{NC}\n")

    if output_file is not None:
        output_file = Path(output_file)
        with output_file.open("w") as f:
            f.write(full_code)
        output = []
    elif language == "bash":
        result = subprocess.run(
            full_code,
            capture_output=True,
            text=True,
            shell=True,
        )
        output = result.stdout.split("\n")
    else:
        msg = "Specify 'output_file' for non-Python/Bash languages."
        raise ValueError(msg)

    if verbose:
        print(f"\n🐓 {BOLD}{GREEN}output:{NC}")
        print(f"\n{CYAN}{output}\n{NC}")

    return output


def _bold(text: str) -> str:
    """Format a string as bold."""
    bold = "\033[1m"
    reset = "\033[0m"
    return f"{bold}{text}{reset}"


@dataclass
class ProcessingState:
    """State of the processing of a Markdown file."""

    section: Literal[
        "🐖",
        "👨‍🌾",
        "🐓"
    ] = "👨‍🌾"
    code: list[str] = field(default_factory=list)
    context: dict[str, Any] = field(default_factory=dict)
    skip_code_block: bool = False
    output: list[str] | None = None
    new_lines: list[str] = field(default_factory=list)
    in_table: bool = False,
    backtick_options: dict[str, Any] = field(default_factory=dict)

    def process_line(self, line: str, *, verbose: bool = False) -> None:
        """Process a line of the Markdown file."""
        if line.strip().startswith("|==="):
            self.in_table = not self.in_table
        ################################################################################
        if (self.section == "👨‍🌾" and
                line.lstrip().startswith("<!--") and
                line.find("🐖") != -1):
            self.section = "🐖"
            self.code.append(remove_md_comment(line.strip()).replace("🐖", ""))
            if line.rstrip().endswith("-->"):
                self._process_chicken_code(verbose=verbose)
                self._process_output_start(line)
        elif self.section == "🐖":
            if line.lstrip() == "-->":
                self._process_chicken_code(verbose=verbose)
                self._process_output_start("-->")
            else:
                self.code.append(line)
        if self.section != "🐓":
            self.new_lines.append(line)
        elif self.section == "🐓" and line == "++++":
            self.section = "👨‍🌾"

    def _process_output_start(self, line: str) -> None:
        self.section = "🐓"
        assert isinstance(
            self.output,
            list,
        ), f"Output must be a list, not {type(self.output)}, line: {line}"
        if not self.in_table:
            new_output = []
            for c in self.output:
                new_output.append(c.replace("\\|", "|").replace("|", "\\|"))
            new_line = line.replace("\\|", "|").replace("|", "\\|")
            self.new_lines = [*self.new_lines, new_line, *new_output]
        else:
            self.new_lines = [*self.new_lines, line, *self.output]
        self.output = None  # Reset output after processing end of the output section

    def _process_chicken_code(self, *, verbose: bool) -> None:
        print(f"code: {self.code}")
        self.output = execute_code(
            self.code,
            self.context,
            "bash",
            output_file=self.backtick_options.get("filename"),
            verbose=verbose,
        )
        self.section = "👨‍🌾"
        self.code = []
        self.backtick_options = {}


def process_markdown(content: list[str], *, verbose: bool = False) -> list[str]:
    """Executes code blocks in a list of Markdown-formatted strings and returns the modified list.

    Parameters
    ----------
    content
        A list of Markdown-formatted strings.
    verbose
        If True, print every line that is processed.

    Returns
    -------
    list[str]
        A modified list of Markdown-formatted strings with code block output inserted.
    """
    assert isinstance(content, list), "Input must be a list"
    state = ProcessingState()

    for i, line in enumerate(content):
        if verbose:
            nr = _bold(f"line {i:4d}")
            print(f"{nr}: {line}")
        state.process_line(line, verbose=verbose)

    return state.new_lines


def update_markdown_file(
        input_filepath: Path | str,
        output_filepath: Path | str | None = None,
        *,
        verbose: bool = False,
) -> None:
    """Rewrite a Markdown file by executing and updating code blocks."""
    if isinstance(input_filepath, str):  # pragma: no cover
        input_filepath = Path(input_filepath)
    with input_filepath.open() as f:
        original_lines = [line.rstrip("\n") for line in f.readlines()]
    if verbose:
        print(f"Processing input file: {input_filepath}")
    new_lines = process_markdown(original_lines, verbose=verbose)
    updated_content = "\n".join(new_lines).rstrip() + "\n"
    if verbose:
        print(f"Writing output to: {output_filepath}")
    output_filepath = (
        input_filepath if output_filepath is None else Path(output_filepath)
    )
    with output_filepath.open("w") as f:
        f.write(updated_content)
    if verbose:
        print("Done!")


def main() -> None:
    """Parse command line arguments and run the script."""
    parser = argparse.ArgumentParser(
        description="Automatically update Markdown files with code block output.",
    )
    parser.add_argument(
        "input",
        type=str,
        help="Path to the input Markdown file.",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=str,
        help="Path to the output Markdown file. (default: overwrite input file)",
        default=None,
    )
    parser.add_argument(
        "-d",
        "--verbose",
        action="store_true",
        help="Enable debugging mode (default: False)",
    )
    parser.add_argument(
        "-v",
        "--version",
        action="version",
        version=f"%(prog)s {__version__}",
    )

    args = parser.parse_args()

    input_filepath = Path(args.input)
    output_filepath = Path(args.output) if args.output is not None else input_filepath
    update_markdown_file(input_filepath, output_filepath, verbose=args.verbose)


if __name__ == "__main__":
    main()
