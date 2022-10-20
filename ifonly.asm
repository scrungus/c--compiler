.globl main
.text
main:
# Creating new frame
addiu $sp, $sp -12
sw $ra, 4($sp)
li $t1, 12
sw $t1, 0($sp)
# End of creating frame
#saving global frame
# Saving frame
# End of saving frame
jal _main
#print integer result
move $a0 $v1
li $v0 1
syscall
li $a0 10
li $v0 11
syscall
li $v0 10
syscall
_main:
# Creating new frame
addiu $sp, $sp -16
sw $ra, 4($sp)
li $t1, 16
sw $t1, 0($sp)
# End of creating frame

li $t0,3

li $t2,3
bne $t0 $t2 L1
li $v1 1
addiu $sp, $sp 16
jr $ra
L1:
li $v1 0
addiu $sp, $sp 16
jr $ra
