
clang++ tick_bitmap_test.cpp -o a -g -std=c++17 -O3 -Wall -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fsanitize=local-bounds -fsanitize=float-divide-by-zero -fsanitize=unsigned-integer-overflow -fsanitize=nullability-arg -fsanitize=nullability-assign -fsanitize=nullability-return && ./a>out
