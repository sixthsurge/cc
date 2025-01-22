global main
section .data
section .text
f:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov qword [rbp-8], rdi
	mov eax, 1
	cdqe
	mov rbx, rax
	mov rax, qword [rbp-8]
	add rax, rbx
	leave
	ret
main:
	push rbp
	mov rbp, rsp
	sub rsp, 32
	mov word [rbp-2], 3
	movsx rax, word [rbp-2]
	mov qword [rbp-16], rax
	mov rdi, qword [rbp-16]
	call f
	mov qword [rbp-24], rax
	mov rdi, qword [rbp-16]
	call f
	mov rbx, rax
	mov rax, qword [rbp-24]
	add rax, rbx
	leave
	ret
