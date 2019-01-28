.text
.globl go
go:
# save callee-saved registers
pushq %rbx
pushq %rbp
pushq %r12
pushq %r13
pushq %r14
pushq %r15

call _main
# restore callee-saved registers and return
popq %r15
popq %r14
popq %r13
popq %r12
popq %rbp
popq %rbx
retq
_main:
subq $0, %rsp #Allocate locals
movq $5, %rdi
movq $7, %rsi
cmpq %rsi, %rdi
je _choice1
jmp _choice2
_choice1:
call allocate
movq 0(%rax), %rdi
movq $_encodeArraySize_ret, -8(%rsp)
subq $8, %rsp
jmp _encodeArraySize
_encodeArraySize_ret:
movq %rax, %rdi
call print
_choice2:
movq $5, %rdi
movq $7, %rsi
movq $_generate3Darray_ret, -8(%rsp)
subq $8, %rsp
jmp _generate3Darray
_generate3Darray_ret:
addq $0, %rsp
retq
_encodeArraySize:
subq $0, %rsp #Allocate locals
salq $1, %rdi
addq $1, %rdi
movq %rdi, %rax
addq $0, %rsp
retq
_generate3Darray:
subq $0, %rsp #Allocate locals
call allocate
movq $5, %rdi
movq %rax, %rsi
call allocate
movq $5, %rdi
movq %rax, %rsi
call allocate
movq %rax, %rdi
call print
addq $0, %rsp
retq
