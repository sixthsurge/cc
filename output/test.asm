global main
section .data
section .text
example_function:
	push rbp
	mov rbp, rsp
main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
