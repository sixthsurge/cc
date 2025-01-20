global main
section .data
section .text
g:
	push rbp
	mov rbp, rsp
	sub rsp, 48
	mov dword [rbp-4], edi
	mov dword [rbp-8], esi
	mov dword [rbp-12], edx
	mov dword [rbp-16], ecx
	mov dword [rbp-20], r8d
	mov dword [rbp-24], r9d
	mov eax, dword [rbp+16]
	mov dword [rbp-28], eax
	mov eax, dword [rbp+24]
	mov dword [rbp-32], eax
	mov eax, dword [rbp+32]
	mov dword [rbp-36], eax
	mov ebx, dword [rbp-36]
	mov eax, dword [rbp-32]
	add eax, ebx
	mov ebx, eax
	mov eax, dword [rbp-28]
	add eax, ebx
	mov ebx, eax
	mov eax, dword [rbp-24]
	add eax, ebx
	mov ebx, eax
	mov eax, dword [rbp-20]
	add eax, ebx
	mov ebx, eax
	mov eax, dword [rbp-16]
	add eax, ebx
	mov ebx, eax
	mov eax, dword [rbp-12]
	add eax, ebx
	mov ebx, eax
	mov eax, dword [rbp-8]
	add eax, ebx
	mov ebx, eax
	mov eax, dword [rbp-4]
	add eax, ebx
	leave
	ret
main:
	push rbp
	mov rbp, rsp
	push 9
	push 8
	push 7
	mov r9d, 6
	mov r8d, 5
	mov ecx, 4
	mov edx, 3
	mov esi, 2
	mov edi, 1
	call g
	add rsp, 24
	leave
	ret
