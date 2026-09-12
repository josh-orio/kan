PROJECT := kan
BUILD_DIR := build
EXECUTABLE := $(BUILD_DIR)/$(PROJECT)

LLVM_PREFIX := $(shell brew --prefix llvm@20)

.PHONY: all build clean run

all: build run

build:
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR)  && \
	PROJECT_NAME=$(PROJECT) \
	cmake \
	-DCMAKE_C_COMPILER=$(LLVM_PREFIX)/bin/clang \
	-DCMAKE_CXX_COMPILER=$(LLVM_PREFIX)/bin/clang++ \
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
