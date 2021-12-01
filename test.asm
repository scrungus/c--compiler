.globl main
.text
main:
li $a0 8
li $v0 9
syscall
move $fp $v0
li $t0,0

li $t1,1
li $t2,1
beq $t1 $t2 L1
li $t1,3
move $t0 $t1
j L2
L1:
li $t0,1

L2:
move $v1 $t0

move $a0 $v1
li $v0 1
syscall
li $a0 10
li $v0 11
syscall
li $v0 10
syscall
