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
    "ollama",
    #"work-medium",
    "work-small",
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
    "largearr"
]

ITERATIONS = [10]
VERSION = [1]

rule all:
    input:
        expand("out/{version}_{trace}_{alg}_{iter}_{inv}.parquet",
        version=VERSION,
        trace=TRACES,
        alg=ALGS,
        iter=ITERATIONS,
        inv=INVALIDATION_METHODS
        )

rule run_single:
    output:
        temp("tmp/{version}_{trace}_{alg}_{iter}_{inv}.csv")
    threads: 1
    params:
        trace_file = lambda wildcards: f"{TRACES_DIR / TRACES_ABBR[wildcards.trace]}"
    shell:
        "unxz -c {params.trace_file} | " 
        "bin/{wildcards.alg} {wildcards.inv} csv {wildcards.iter} > {output}"

rule conv_to_parquet:
    input:
        "tmp/{version}_{trace}_{alg}_{iter}_{inv}.csv"
    output:
        "out/{version}_{trace}_{alg}_{iter}_{inv}.parquet"
    priority: 10
    run:
        pl.read_csv(input[0]).with_columns([
            pl.lit(int(wildcards.version)).alias("version"),
            pl.lit(wildcards.trace).alias("trace"),
            pl.lit(wildcards.alg).alias("alg"),
            pl.lit(wildcards.iter).alias("iter"),
            pl.lit(wildcards.inv).alias("inv"),
        ]).write_parquet(output[0])
    


