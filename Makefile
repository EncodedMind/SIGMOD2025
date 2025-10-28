CXX = clang++
CXXFLAGS = -std=c++20 -Wall -g

INCLUDES = -Iinclude -Itests

TARGETS = robinhood hopscotch cuckoo
CATCH_SRC = tests/catch_amalgamated.cpp
CATCH_HDR = tests/catch_amalgamated.hpp

ROBINHOOD_SRC = \
			tests/robinhoodtests.cpp \
			robinhood.cpp \
			$(CATCH_SRC)

ROBINHOOD_HDR = \
			include/robinhood.hpp \
			$(CATCH_HDR)

# Compile all of them or seperatelly
all: $(TARGETS)

robinhood: $(ROBINHOOD_SRC) $(ROBINHOOD_HDR)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(ROBINHOOD_SRC) -o $@

hopscotch: hopscotch.cpp
	@$(CXX) $(CXXFLAGS) $< -o $@

cuckoo: cuckoo.cpp
	@$(CXX) $(CXXFLAGS) $< -o $@

# Run all of them or seperatelly
run: run-robinhood run-hopscotch run-cuckoo

run-robinhood: robinhood
	@echo "\n---- Running Robin Hood Hashing Tests----\n"
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
