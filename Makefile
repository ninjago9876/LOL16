CC = gcc
COMPILER_LIBS = -lm -lpcre2-8
CFLAGS = -msse2 -march=native -Wall -Wextra

TARGET_EXECUTABLE = lol16
SRC_DIR = src
BUILD_DIR = build

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

all: $(TARGET_EXECUTABLE)

$(TARGET_EXECUTABLE): $(OBJ_FILES)
	@echo "Linking Started"
	@$(CC) -o $@ $^ $(CFLAGS) $(COMPILER_LIBS) 
	@echo "Linking Complete"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compilation Started for $<"
	@$(CC) -o $@ -c $< $(COMPILER_LIBS) $(CFLAGS)
	@echo "Compilation Complete for $<"

clean:
	@rm -f $(OBJ_FILES) $(TARGET_EXECUTABLE)
	@echo "Cleaned up object files and executable"
