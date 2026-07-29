trace_name=ollama
trace=./images/REFERENCE_ollama_qwen25_coder_32b.page-images.xz

binaries=(lz4 WKdm zlib WK64 zstd lzo)

for bin in "${binaries[@]}"; do
    echo -n "Starting ${bin}..."
    cat "./correctness/REFERENCE_ollama_qwen25_coder_32b.page-images" | "./bin/$bin" "REFERENCE_ollama" "none" "none" "csv"  > "./correctness/GOLD_${trace_name}_${bin}.csv"
    echo "done."
done