.text

.globl deep_call
.globl deep_return

deep_call:
    # movl 8(%esp), %eax # target*
    # movl 12(%esp), %ecx # param*
    movl 16(%esp), %edx # saveaddr**
    subl $24, %esp
    movl %ebx, 4(%esp)
    movl %esi, 8(%esp)
    movl %edi, 12(%esp)
    movl %ebp, 16(%esp)
    movl %esp, (%edx)

    pushl 28(%esp) # param*
    call *24(%esp) # target*

    addl $28, %esp

    movl $0x01, %eax
    ret

deep_return:
    movl 8(%esp), %esp # loadaddr*
    movl 4(%esp), %ebx
    movl 8(%esp), %esi
    movl 12(%esp), %edi
    movl 16(%esp), %ebp
    addl $24, %esp
    xorl %eax, %eax
    ret
