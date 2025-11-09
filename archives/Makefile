CXX = clang++
CXXFLAGS = -Wall -g

TARGETS = robinhood hopscotch cuckoo

# Compile all of them or seperatelly
all: $(TARGETS)

robinhood: robinhood.cpp
	@$(CXX) $(CXXFLAGS) $< -o $@

hopscotch: hopscotch.cpp
	@$(CXX) $(CXXFLAGS) $< -o $@

cuckoo: cuckoo.cpp
	@$(CXX) $(CXXFLAGS) $< -o $@

# Run all of them or seperatelly
run: run-robinhood run-hopscotch run-cuckoo

run-robinhood: robinhood
	@echo "\n---- Running Robin Hood Hashing ----\n"
	@./$<

run-hopscotch: hopscotch
	@echo "\n---- Running Hopscotch Hashing ----\n"
	@./$<

run-cuckoo: cuckoo
	@echo "\n---- Running Cuckoo Hashing ----\n"
	@./$<

# Clean up
clean:
	@rm -f $(TARGETS)
