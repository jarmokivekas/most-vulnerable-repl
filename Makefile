

shellcode:
	gcc build/expression.c -g -S -o build/expression.s -O0 -fno-stack-protector
	gcc -Xlinker -T repl.linker build/expression.s -g -o build/expression.o -c -O0 -fno-stack-protector
	objdump -d build/expression.o | grep '000000* <expression>:' -A 10000 | grep -v expression > build/expression.objdump
	cat build/expression.objdump | tr '\t' '@' | cut '-d@' -f 2 | xxd -r -p > build/expression.shellcode
	cat build/expression.objdump
	hexdump -C build/expression.shellcode
	ls -la build

repl:
	gcc src/repl.c -o build/repl.out -z execstack -g
	objdump -d build/repl.out > build/repl.out.objdump
	nm -g build/repl.out | egrep '^[0-9a-e]{8,} T' |awk '{printf "PROVIDE(%s = 0x%s);\n",$$3,$$1}' > build/repl.linker
run: repl
	./build/repl.out
clean:
	rm -rf build/*
