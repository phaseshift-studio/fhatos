# Copyright (c) 2023, Bas Nijholt
# All rights reserved.
"""Markdown Code Runner.

Automatically update Markdown files with code block output.

This script is part of the 'markdown-code-runner' package available on GitHub:
https://github.com/basnijholt/markdown-code-runner

Add code blocks between <!-- CODE:START --> and <!-- CODE:END --> in your Markdown file.
The output will be inserted between <!-- OUTPUT:START --> and <!-- OUTPUT:END -->.

Example:
-------
<!-- CODE:START -->
<!-- print('Hello, world!') -->
<!-- CODE:END -->
<!-- OUTPUT:START -->
This will be replaced by the output of the code block above.
<!-- OUTPUT:END -->

Alternatively, you can add a <!-- CODE:SKIP --> comment above a code block to skip execution.

Another way is to run code blocks in triple backticks:
```python markdown-code-runner
print('Hello, world!')
```
Which will print the output of the code block between the output markers:
<!-- OUTPUT:START -->
This will be replaced by the output of the code block above.
<!-- OUTPUT:END -->

You can also run bash code blocks:
```bash markdown-code-runner
echo "Hello, world!"
```
Which will similarly print the output of the code block between next to the output markers.
"""
from __future__ import annotations

import argparse
import contextlib
import io
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import TYPE_CHECKING, Any

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
                new_code.append(new_code.pop().removesuffix("/\"") + "\n        " + c + "\"")
            else:
                new_code.append("\"" + c + "\"")
        cat = c.endswith("/")
    full_code = " ".join(new_code)

    if verbose:
        print(_bold(f"\nExecuting code {language} block:"))
        print(f"\n{full_code}\n")

    if output_file is not None:
        output_file = Path(output_file)
        with output_file.open("w") as f:
            f.write(full_code)
        output = []
    elif language == "python":
        with io.StringIO() as string, contextlib.redirect_stdout(string):
            exec(full_code, context)  # noqa: S102
            output = string.getvalue().split("\n")
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
        print(_bold("Output:"))
        print(f"\n{output}\n")

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
        "🐓",
        "👨‍🌾",
        "🦆"
    ] = "👨‍🌾"
    code: list[str] = field(default_factory=list)
    original_output: list[str] = field(default_factory=list)
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
        if line.find("🐖") is not -1 or (self.section is "🐓" and line.strip() == "-->"):
            self._process_chicken_code(line, verbose=verbose)
            self.section = "👨‍🌾"
            self.code = []
            self.backtick_options = {}
            self._process_output_start(line)
        elif line.find("🦆") is not -1:
            self._process_output_end()
        elif self.section is "🐓" or line.find("🐓") is not -1:
            self.section = "🐓"
            self.code.append(remove_md_comment(line.replace("🐓", "")))
        elif self.section == "🦆":
            self.original_output.append(line)
        if self.section != "🦆":
            self.new_lines.append(line)

    def _process_output_start(self, line: str) -> None:
        self.section = "🦆"
        if not self.skip_code_block:
            assert isinstance(
                self.output,
                list,
            ), f"Output must be a list, not {type(self.output)}, line: {line}"
            if not self.in_table:
                new_output = []
                for c in self.output:
                    new_output.append(c.replace("|", "\\|"))
                new_line = line.replace("|", "\\|")
                self.new_lines.extend([new_line, *new_output])
            else:
                self.new_lines.extend([line, *self.output])
        else:
            if not self.in_table:
                new_line = line.replace("|", "\\|")
                self.original_output.append(new_line)
            else:
                self.original_output.append(line)

    def _process_output_end(self) -> None:
        self.section = "👨‍🌾"
        if self.skip_code_block:
            self.new_lines.extend(self.original_output)
            self.skip_code_block = False
        self.original_output = []
        self.output = None  # Reset output after processing end of the output section

    def _process_chicken_code(self, line: str, *, verbose: bool) -> None:
        print(self.code)
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
