from pathlib import Path
from itertools import product
import json
import polars as pl

# Configuration
TRACES_DIR = Path("~sfkaplan/traces")
TRACES = [
    #"build-llvm",
    #"spec-all",
    #"login",
    #"ollama",
    "work-medium",
    #"work-small",
]
TRACES_ABBR = {
    "build-llvm" : "2026-06-22-build-llvm.page_image.xz",
    "spec-all" : "2026-06-25-spec-all.page_image.xz",
    "login" : "login.images.xz",
    "ollama" : "ollama_qwen25_coder_32b.page-images.xz",
    "work-medium" : "work-medium.images.xz",
    "work-small" : "work-small.images.xz",
}

ALGS = [
    "lz4", 
    #"lzo", 
    #"WK64", 
    "WKdm", 
    #"zlib", 
    "zstd"
]

INVALIDATION_METHODS = [
    "none",
    "clflush",
    "largearr",
    #"randlargearr"
]

ITERATIONS = 4
VERSION = 1

rule all:
    input:
        expand("out/{version}_{trace}_{alg}.parquet",
        version=VERSION,
        trace=TRACES,
        alg=ALGS)

rule run_single:
    output:
        temp("tmp/{version}_{trace}_{alg}_{iter}_{inv}.csv")
    threads : 1
    params:
        trace_file = lambda wildcards: f"{TRACES_DIR / TRACES_ABBR[wildcards.trace]}"
    shell:
        "unxz -c {params.trace_file} | " 
        "bin/{wildcards.alg} {wildcards.inv} csv {wildcards.iter} > {output}"

rule aggregate:
    input:
        expand(
            "tmp/{{version}}_{{trace}}_{{alg}}_{iter}_{inv}.csv",
            iter=ITERATIONS,
            inv=INVALIDATION_METHODS
        )
    output:
        "out/{version}_{trace}_{alg}.parquet"
    run:
        dfs = []
        for filepath in input:
            dfs.append(pl.read_csv(filepath).with_columns([
                pl.lit(int(wildcards.version)).alias("version"),
                pl.lit(wildcards.trace).alias("trace"),
                pl.lit(wildcards.alg).alias("alg")
            ]))
        pl.concat(dfs).write_parquet(output[0])