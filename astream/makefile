CC=g++
CFLAGS=-Wall
OUT_DIR=bin
SRC_DIR=sources

all:
	mkdir -p $(OUT_DIR)
	cp astream.conf $(OUT_DIR)/astream.conf
	$(CC) $(CFLAGS) $(SRC_DIR)/astream.cpp $(SRC_DIR)/HttpClient.cpp -o $(OUT_DIR)/astream -std=c++11 -lboost_program_options -lboost_system -lboost_thread -lcurl -ljson_spirit

clean:
	rm -rf $(OUT_DIR)

