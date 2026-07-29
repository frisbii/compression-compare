trace_name=ollama
trace=./images/REFERENCE_ollama_qwen25_coder_32b.page-images.xz

binaries=(lz4 WKdm zlib WK64 zstd lzo)
#binaries=(lz4)
for bin in "${binaries[@]}"; do
    echo -n "Starting ${bin}..."
    refdata="./correctness/GOLD_${trace_name}_${bin}.csv"
    cat "./correctness/REFERENCE_ollama_qwen25_coder_32b.page-images" | "./bin/$bin" | cut -d',' -f1-3 | cmp - <(cut -d',' -f1-3 ${refdata}) 
    echo "done."
done