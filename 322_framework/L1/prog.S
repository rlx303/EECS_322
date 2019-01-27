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

   call _go

   # restore callee-saved registers and return
   popq %r15
   popq %r14
   popq %r13
   popq %r12
   popq %rbp
   popq %rbx
   retq


_go:
   movq $10, %rdi
   movq $2, %rsi
   movq $_div_ret_1, -8(%rsp)
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _easy_divisible        # Call to :easy_divisible
_div_ret_1:
   salq $1, %rax
   inc %rax
   movq %rax, %rdi
   call print  # runtime system call
   movq $10, %rdi
   movq $3, %rsi
   movq $_div_ret_2, -8(%rsp)
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _easy_divisible        # Call to :easy_divisible
_div_ret_2:
   salq $1, %rax
   inc %rax
   movq %rax, %rdi
   call print  # runtime system call
   ret


_easy_divisible:
   movq $1, %rsi
_START:
   cmpq $0, %rdi
   je _DONE
   subq %rsi, %rdi
   cmpq $0, %rdi
   jl _NOO
   jmp _START
_NOO:
   movq $0, %rsi
_DONE:
   movq %rsi, %rax
   ret

