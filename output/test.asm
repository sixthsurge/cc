global main
section .text
main:
	push rbp
	mov rbp, rsp
	sub rsp, 64
	mov dword [rbp-4], 33
	mov dword [rbp-8], 20
	mov eax, dword [rbp-4]
	leave
	ret
