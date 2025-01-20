global main
section .data
section .text
example_function:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov dword [rbp-4], edi
	mov eax, 5
	leave
	ret
main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov dword [rbp-4], 3
	mov eax, dword [rbp-4]
	leave
	ret
