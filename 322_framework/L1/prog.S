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
   movq $5, %rdi
   call print  # runtime system call
   movq %rdi, %r10
   movq $11, %rdi
   movq $0, %rsi
   call allocate  # runtime system call
   movq %rax, %r11
   movq $_odd_num_1, -8(%rsp)
   movq $2, %rdi
   subq $8, %rsp      # Allocate extra arguments on the stack as well as the return address 
   jmp _odd_num        # Call to :odd_num
_odd_num_1:
   movq %rax, 8(%r11)
   movq $5, 16(%r11)
   movq $7, 24(%r11)
   movq $9, 32(%r11)
   movq $11, 40(%r11)
   movq 8(%r11), %rsi
   movq 16(%r11), %rdi
   subq %rsi, %rdi
   inc %rdi
   addq 16(%r11), %rdi
   movq 24(%r11), %rcx
   salq %cl, %rdi
   andq 32(%r11), %rdi
   subq 40(%r11), %rdi
   addq $3, %rdi
   salq $1, %rdi
   inc %rdi
   call print  # runtime system call
   ret


_odd_num:
   movq $1, %rax
_start:
   subq $1, %rdi
   cmpq $0, %rdi
   jle _done
   addq $2, %rax
   jmp _start
_done:
   ret

