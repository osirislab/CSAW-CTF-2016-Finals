dev: ed.c vector.c
		gcc -g -o ed -Wl,-z,relro,-z,now -ffast-math -Werror -Wall -Wextra -Wfloat-equal -Wshadow -Wcast-align -Wunreachable-code -Wunused-variable main.c ed.c vector.c -ldl
release:
		gcc -o ed -fPIE -pie -fstack-protector -s -Wl,-z,relro,-z,now -ffast-math -Werror -Wall -Wextra -Wfloat-equal -Wshadow -Wcast-align -Wunreachable-code -Wunused-variable main.c ed.c vector.c -ldl
clean:
		rm ed
