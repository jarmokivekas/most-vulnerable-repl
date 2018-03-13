

shellcode:
	gcc build/expression.c -S -o build/expression.s -O0 -fno-stack-protector
	gcc build/expression.s -o build/expression.o -c -O0 -fno-stack-protector
	objdump -d build/expression.o | grep '000000* <expression>:' -A 10000 | grep -v expression > build/expression.objdump
	cat build/expression.objdump | tr '\t' '@' | cut '-d@' -f 2 | xxd -r -p > build/expression.shellcode
	cat build/expression.objdump
	hexdump -C build/expression.shellcode
	ls -la build

repl:
	gcc src/repl.c -o build/repl.out -z execstack -g

run: repl
	./build/repl.out
clean:
	rm -rf build/*
