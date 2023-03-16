clear && clang++ main.cpp -o a -g -std=c++17 -O3 -Wall -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fsanitize=local-bounds -fsanitize=float-divide-by-zero -fsanitize=unsigned-integer-overflow -fsanitize=nullability-arg -fsanitize=nullability-assign -fsanitize=nullability-return -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/libmongoc-1.0 \
  -I/usr/local/include/bsoncxx/v_noabi -I/usr/local/include/libbson-1.0 \
  -L/usr/local/lib -lmongocxx -lbsoncxx -D VALIDATE && ./a
