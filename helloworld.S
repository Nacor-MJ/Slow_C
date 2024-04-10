	.text
    .intel_syntax noprefix
	.globl	main
main:
	pushq	rbp
	movq	rbp, rsp 
	subq	rsp, 32
	leaq	rcx, [.LC0 + rip]
    mov     rdx, 69
	call	printf
	mov	    eax, 25
	addq	rsp, 32
	popq	rbp
	ret
.LC0:
	.ascii "Hello, World! and %d\n\0"
