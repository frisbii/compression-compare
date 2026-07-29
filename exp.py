import itertools

from math import prod
from pathlib import Path
import subprocess
from tqdm import tqdm


TRACES_DIR = Path("~sfkaplan/traces")

def main():
    traces = [
        "ollama_qwen25_coder_32b.page-images.xz"
    ]
    traces_short = {
        "ollama_qwen25_coder_32b.page-images.xz" : "ollama"
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

        runstr = f"unxz -c {trace_path} | {bin} {trace_name} {comp_invalidation_method} {decomp_invalidation_method} 'sql' | sqlite3 data.db"
        #print(runstr)

        res = subprocess.run(
            runstr,
            shell=True,
            text=True
        )

main()