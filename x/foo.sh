set -e

# Resolve Apple clang and SDK
CLANG="$(xcrun -f clang)"
SDKROOT="$(xcrun --sdk macosx --show-sdk-path)"

# Create test file
cat > /tmp/asan_test.c <<'EOF'
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int *p = malloc(sizeof(int));
    free(p);
    return p[0];  // use-after-free (ASan must catch this)
}
EOF

# Compile with ASan
"$CLANG" \
  -isysroot "$SDKROOT" \
  -O0 -g -fno-omit-frame-pointer \
  -fsanitize=address \
  /tmp/asan_test.c \
  -o /tmp/asan_test

# Show ASan linkage
echo "Linked ASan dylib:"
otool -L /tmp/asan_test | grep asan || echo "❌ no ASan dylib"

# Run
echo
echo "Running binary:"
MallocNanoZone=0 /tmp/asan_test

