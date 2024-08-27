#CFLAGS=-Wall -Werror -Wextra
CFLAGS = -Wall -Werror -Wextra -std=c11

all: clean s21_decimal.a test gcov_report

ubuntu: clean s21_decimal.a test_ubuntu gcov_report

test: *.c
	gcc -o test $(CFLAGS) --coverage *.c -lcheck -lpthread -lm  -I /usr/local/include -L/usr/local/lib
	
test_ubuntu: *.c
	gcc -o test $(CFLAGS) --coverage *.c -lcheck -lm -lpthread -lsubunit
	
s21_decimal.a: s21_add.o s21_compare.o s21_conversion.o s21_decimal_div.o s21_decimal_mod.o s21_decimal_mul.o s21_decimal_sub.o s21_other_func.o supp_func.o
	ar rc s21_decimal.a s21_add.o s21_compare.o s21_conversion.o s21_decimal_div.o s21_decimal_mod.o s21_decimal_mul.o s21_decimal_sub.o s21_other_func.o supp_func.o
	ranlib s21_decimal.a
	

s21_add.o: s21_add.c
	gcc $(CFLAGS) -c s21_add.c

s21_compare.o: s21_compare.c
	gcc $(CFLAGS) -c s21_compare.c
	
s21_conversion.o: s21_conversion.c
	gcc $(CFLAGS) -c s21_conversion.c

s21_decimal_div.o: s21_decimal_div.c
	gcc $(CFLAGS) -c s21_decimal_div.c

s21_decimal_mod.o: s21_decimal_mod.c
	gcc $(CFLAGS) -c s21_decimal_mod.c

s21_decimal_mul.o: s21_decimal_mul.c
	gcc $(CFLAGS) -c s21_decimal_mul.c

s21_decimal_sub.o: s21_decimal_sub.c
	gcc $(CFLAGS) -c s21_decimal_sub.c

s21_other_func.o: s21_other_func.c
	gcc $(CFLAGS) -c s21_other_func.c

supp_func_print.o: supp_func_print.c
	gcc $(CFLAGS) -c supp_func_print.c

supp_func.o: supp_func.c
	gcc $(CFLAGS) -c supp_func.c

	
gcov_report: test
	chmod +x *
	./test


check:
	cp ../materials/linters/.clang-format ../src/.clang-format
	clang-format -i *.c
	clang-format -i *.h
	

clean:
	rm -rf *.o *.gcda *.a *.html *.gcno *.css
	rm -f test
	rm -f test_ubuntu
	
git:
	git add *.c
	git add *.h
	git add Makefile
	git commit -m "add from make"
	git push