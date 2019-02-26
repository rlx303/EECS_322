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
subq $8, %rsp #Allocate locals
_call_label0:
movq $21, %rdi
movq %rdi, 0(%rsp)
movq $21, %rdi
sarq $1, %rdi
movq $21, %rsi
sarq $1, %rsi
imulq %rsi, %rdi
addq $3, %rdi
salq $1, %rdi
addq $1, %rdi
movq %rdi, %rdi
movq $1, %rsi
call allocate
movq %rax, %rsi
movq %rsi, %rdi
addq $8, %rdi
movq $5, 0(%rdi)
addq $8, %rdi
movq $21, 0(%rdi)
addq $8, %rdi
movq $21, 0(%rdi)
movq %rsi, %rdi
movq 0(%rsp), %rsi
movq %rsi, %rsi
movq $_call_label1, -8(%rsp)
subq $8, %rsp
jmp _checkMatrix
_call_label1:
movq %rax, %rdi
movq %rdi, %rdi
call print
addq $8, %rsp
retq
_checkMatrix:
subq $32, %rsp #Allocate locals
movq %rdi, %rdi
movq %rdi, 24(%rsp)
movq %rsi, %rdi
movq %rdi, 16(%rsp)
_call_label_ciao_2:
movq $0, %rsi
imulq $8, %rsi
addq $16, %rsi
movq 24(%rsp), %rdi
addq %rdi, %rsi
movq 0(%rsi), %rdi
movq %rdi, 8(%rsp)
movq $1, %rsi
imulq $8, %rsi
addq $16, %rsi
movq 24(%rsp), %rdi
addq %rdi, %rsi
movq 0(%rsi), %rdi
movq %rdi, 0(%rsp)
movq 8(%rsp), %rdi
movq %rdi, %rdi
call print
movq 0(%rsp), %rdi
movq %rdi, %rdi
call print
movq 8(%rsp), %rdi
movq %rdi, %r9
sarq $1, %r9
movq 0(%rsp), %rdi
movq %rdi, %rcx
sarq $1, %rcx
jmp _call_label_ciao_3
_call_label_ciao_3:
movq $1, %rdx
movq $0, %r8
jmp _call_label_ciao_4
_call_label_ciao_4:
cmpq %r9, %r8
setl %dil
movzbq %dil, %rdi
cmpq $1, %rdi
je _call_label_ciao_5
jmp _call_label_ciao_11
_call_label_ciao_5:
movq $0, %rsi
jmp _call_label_ciao_6
_call_label_ciao_6:
cmpq %rcx, %rsi
setl %dil
movzbq %dil, %rdi
cmpq $1, %rdi
je _call_label_ciao_7
jmp _call_label_ciao_9
_call_label_ciao_7:
movq 24(%rsp), %rdi
movq 24(%rdi), %rax
sarq $1, %rax
movq $0, %rdi
movq %r8, %r10
imulq %rax, %r10
addq %r10, %rdi
movq %rsi, %rax
addq %rax, %rdi
imulq $8, %rdi
addq $32, %rdi
movq 24(%rsp), %rax
movq %rax, %rax
addq %rdi, %rax
movq 0(%rax), %rdi
movq 16(%rsp), %rax
cmpq %rax, %rdi
sete %dil
movzbq %dil, %rdi
cmpq $1, %rdi
je _call_label_ciao_10
jmp _call_label_ciao_8
_call_label_ciao_8:
addq $1, %rsi
jmp _call_label_ciao_6
_call_label_ciao_9:
addq $1, %r8
jmp _call_label_ciao_4
_call_label_ciao_10:
movq $3, %rdx
jmp _call_label_ciao_11
_call_label_ciao_11:
movq %rdx, %rax
addq $32, %rsp
retq
