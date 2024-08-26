

CLEAR_COMMAND := clear  

ht:
	@$(CLEAR_COMMAND)
	@echo "Deleting data.db..."
	-rm data.db
	@echo "Compile ht_main..."
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/ht_main.c ./src/hash_file.c ./src/ht_helper_functions.c -lbf -o ./build/runner -O2


bf:
	@$(CLEAR_COMMAND)  
	@echo "Compile bf_main..."
	gcc -I ./include/ -L ./lib/ -Wl,-rpath,./lib/ ./examples/bf_main.c -lbf -o ./build/runner -O2
