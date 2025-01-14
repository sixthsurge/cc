global main
section .text
main:
	push rbp
	mov rbp, rsp
	sub rsp, 32
	mov dword [rbp-4], 5
	mov dword [rbp-8], 6
	mov eax, dword [rbp-4]
	mov ebx, dword [rbp-8]
	add eax, ebx
	mov dword [rbp-12], eax
	mov eax, 1
	mov ebx, 1
	add eax, ebx
	mov dword [rbp-16], eax
	mov eax, dword [rbp-12]
	mov ebx, dword [rbp-16]
	add eax, ebx
	mov dword [rbp-20], eax
	mov eax, dword [rbp-20]
	mov dword [rbp-4], eax
	mov eax, dword [rbp-4]
	mov ebx, 11
	add eax, ebx
	mov eax, eax
	leave
	ret
