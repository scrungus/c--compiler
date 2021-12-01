.globl main
.text
main:
li $t0,1
li $t1,3
add $t2,$t0,$t1
move $v1 $t2

move $a0 $v1
li $v0 1
syscall
li $a0 10
li $v0 11
syscall
li $v0,10
syscall
