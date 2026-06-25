#trace=~sfkaplan/traces/2026-06-22-build-llvm.page-images.xz
#test=llvm

trace=~sfkaplan/traces/ollama_qwen25_coder_32b.page-images.xz
test=ollama


binaries=(lz4 WKdm zlib)
outdir=data

for bin in "${binaries[@]}"; do
    out="${outdir}/${test}_${bin}.csv"
    printf 'running %s -> %s\n' "$bin" "$out"
    unxz -c "$trace" | "./bin/$bin" > "$out"
done
