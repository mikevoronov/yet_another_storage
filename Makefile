
export BUILD_DIR ?= build

default: release

release:
	mkdir -p ./$(BUILD_DIR) && cd ./$(BUILD_DIR) && cmake ../ -DCMAKE_BUILD_TYPE=Release -DWERROR=$(WERROR) && VERBOSE=1 cmake --build .

debug:
	mkdir -p ./$(BUILD_DIR) && cd ./$(BUILD_DIR) && cmake ../ -DCMAKE_BUILD_TYPE=Debug -DWERROR=$(WERROR) && VERBOSE=1 cmake --build .

tests:
	cd ./$(BUILD_DIR) && make test;

bench:
	@if [ -f ./$(BUILD_DIR)/bench-tests ]; then ./$(BUILD_DIR)/bench-tests; else echo "Please run 'make release' or 'make debug' first" && exit 1; fi

# TODO : clang-coverage tests
#coverage:
#	./scripts/coverage.sh

clean:
	rm -rf ./$(BUILD_DIR)
