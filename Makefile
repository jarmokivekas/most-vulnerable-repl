

shellcode:
	gcc build/expression.c -g -S -o build/expression.s -O0 -fno-stack-protector -z execstack
	gcc -Xlinker -Rbuild/repl.o -c build/expression.c -g -o build/expression.o   -O0 -fno-stack-protector -z execstack
	gcc -Xlinker -Rbuild/repl.o    build/expression.o -g -o build/expression.out -O0 -fno-stack-protector -z execstack
	objdump -d build/expression.out | grep '000000* <expression>:' -A 10000 | grep -v expression > build/expression.objdump
	cat build/expression.objdump | tr '\t' '@' | cut '-d@' -f 2 | xxd -r -p > build/expression.shellcode
	cat build/expression.objdump
	hexdump -C build/expression.shellcode
	ls -la build

repl:
	gcc src/repl.c -c -o build/repl.o   -z execstack -g
	gcc src/repl.c -o build/repl.out -z execstack -g
	objdump -d build/repl.out > build/repl.out.objdump
	# nm -g build/repl.out | egrep 'callback' |awk '{printf "PROVIDE(%s = 0x%s);\n",$$3,$$1}' > build/repl.linker
	# cat build/repl.linker
run: repl
	./build/repl.out
clean:
	rm -rf build/*
