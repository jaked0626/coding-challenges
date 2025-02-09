# jzi: My own compression tool  
See [here](https://codingchallenges.fyi/challenges/challenge-huffman) for problem description


How to compile: 
```bash
mkdir build && cd build
cmake ..
make
```

Test:
```bash
# from the build directory
./jzip.out test.txt # 3.4 MB - > 2.0 MB
./jzip.out test.txt.jzip # 2.0 MB -> 3.4 MB
```