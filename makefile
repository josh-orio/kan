PROJECT := kan
BUILD_DIR := build
EXECUTABLE := $(BUILD_DIR)/$(PROJECT)

# The code needs Clang (it uses #embed), so pick clang 20 per platform:
# macOS -> Homebrew's llvm@20, Linux -> distro clang-20 (e.g. apt install clang-20).
ifeq ($(shell uname -s),Darwin)
LLVM_PREFIX := $(shell brew --prefix llvm@20)
CC_BIN := $(LLVM_PREFIX)/bin/clang
CXX_BIN := $(LLVM_PREFIX)/bin/clang++
else
CC_BIN := $(shell command -v clang-20 || command -v clang)
CXX_BIN := $(shell command -v clang++-20 || command -v clang++)
endif

.PHONY: all build clean run

all: build run

build:
	@test -n "$(CXX_BIN)" || { echo "clang not found: install clang-20 (Linux) or llvm@20 (macOS)"; exit 1; }
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR)  && \
	PROJECT_NAME=$(PROJECT) \
	cmake \
	-DCMAKE_C_COMPILER=$(CC_BIN) \
	-DCMAKE_CXX_COMPILER=$(CXX_BIN) \
	-DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTS=OFF .. && \
	$(MAKE) -j

test:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && \
	PROJECT_NAME=$(PROJECT) cmake -D CMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTS=ON .. && \
	$(MAKE) -j && \
	make test ARGS="--output-on-failure"

debug: build
	@lldb $(EXECUTABLE)

clean:
	@rm -rf $(BUILD_DIR) .cache .kan

run:
	@$(EXECUTABLE)

FORMAT_FILES := $(shell find src include -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' \))

format:
	clang-format -i $(FORMAT_FILES)

format-check:
	clang-format --dry-run --Werror $(FORMAT_FILES)
