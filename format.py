import subprocess
import os

src_dir = "src"

for root, dirs, files in os.walk(src_dir):
    for file in files:
        if file == "bflib_render_gpoly.c" or file == "bflib_render_gpoly_refactor.c":
            file_path = os.path.join(root, file)
            subprocess.run(
                ["clang-format", "--style=file", "--verbose", "-i", file_path]
            )
