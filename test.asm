.globl main
.text
main:
li $a0 12
li $v0 9
syscall
move $fp $v0
li $t0,3

li $t1,3

move $t0,$t0
move $t1,$t1
add $t2,$t0,$t1
move $v1 $t2

move $a0 $v1
li $v0 1
syscall
li $a0 10
li $v0 11
syscall
li $v0 10
syscall
