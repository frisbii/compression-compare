import itertools

from math import prod
from pathlib import Path
import subprocess
from tqdm import tqdm


TRACES_DIR = Path("~sfkaplan/traces")

def main():
    traces = [
        #"2026-06-22-build-llvm.page_image.xz",
        #"2026-06-25-spec-all.page_image.xz",
        #"login.images.xz",
        "ollama_qwen25_coder_32b.page-images.xz",
        #"work-medium.images.xz",
        #"work-small.images.xz",
    ]
    traces_short = {
        "2026-06-22-build-llvm.page_image.xz" : "build-llvm",
        "2026-06-25-spec-all.page_image.xz" : "spec-all",
        "login.images.xz" : "login",
        "ollama_qwen25_coder_32b.page-images.xz" : "ollama",
        "work-medium.images.xz" : "work-medium",
        "work-small.images.xz" : "work-small",
    }

    algorithms = [
        "lz4",
        "lzo",
        "WK64",
        "WKdm",
        "zlib",
        "zstd"
    ]
    
    comp_invalidation_methods = [
        "none",
        "clflush"
    ]

    decomp_invalidation_methods = [
        "none",
        "clflush"
    ]

    product_args = [
        traces,
        algorithms,
        comp_invalidation_methods,
        decomp_invalidation_methods
    ]
    total = prod(len(a) for a in product_args)
    rows = []

    for i, (
        trace, 
        alg,
        comp_invalidation_method,
        decomp_invalidation_method
    ) in enumerate(tqdm(
        itertools.product(*product_args),
        total=total
    )):
        trace_path = TRACES_DIR / trace
        trace_name = traces_short[trace]

        bin = f"./bin/{alg}"

        runstr = f"unxz -c {trace_path} | {bin} {trace_name} {comp_invalidation_method} {decomp_invalidation_method} 'csv' | sqlite3 data.db"
        #print(runstr)

        res = subprocess.run(
            runstr,
            shell=True,
            text=True
        )

main()