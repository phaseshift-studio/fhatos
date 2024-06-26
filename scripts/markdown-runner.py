import os
from markdown_code_runner import update_markdown_file
from subprocess import call

cwd = os.getcwd()
print(cwd)
update_markdown_file(input_filepath="../docs/src/fhatos.md",output_filepath="../docs/src/fhatos_cr.md",verbose=True)