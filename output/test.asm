global main
section .text
example_function:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	mov eax, 5
	leave
	ret
main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov eax, 3
	mov ebx, 3
	add eax, ebx
	mov dword [rbp-4], eax
	mov dword [rbp-8], 4
	mov eax, dword [rbp-4]
	mov ebx, 1
	add eax, ebx
	mov dword [rbp-4], eax
	mov eax, dword [rbp-8]
	mov ebx, 1
	add eax, ebx
	mov dword [rbp-12], eax
	mov eax, dword [rbp-4]
	mov ebx, dword [rbp-12]
	add eax, ebx
	mov eax, eax
	leave
	ret
