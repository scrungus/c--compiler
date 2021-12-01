.globl main
.text
main:
li $a0 8
li $v0 9
syscall
move $fp $v0
li $t0,3

li $t1,3
ble $t0 $t1 L1
li $v1 1
j L2
L1:
li $v1 0
L2:

move $a0 $v1
li $v0 1
syscall
li $a0 10
li $v0 11
syscall
li $v0 10
syscall
