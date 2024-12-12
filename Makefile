CXX = clang++
CXXFLAGS = -Wall -std=c++20 -finput-charset=UTF-8

COMMON_OBJ = cui.o

# target rules
%: %.o $(COMMON_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $< $(COMMON_OBJ)

# object file rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# クリーンアップ
clean:
	rm -f *.o battle map