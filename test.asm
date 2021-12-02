.globl main
.text
x:
# Creating new frame
lw $t0, 4($sp)
addiu $sp, $sp -8
li $t1, 8
li $t2, 1
sw $t1, 0($sp)
addi $t0, $t0 1
sw $t0, 4($sp)
sw $t2, 8($sp)
# End of creating frame
move $t0 $a0
move $t0,$t0
li $t1,1
add $t2,$t0,$t1
move $v1 $t2
jr $ra
main:
# Creating new frame
lw $t0, 4($sp)
addiu $sp, $sp -4
li $t1, 4
li $t2, 0
sw $t1, 0($sp)
addi $t0, $t0 1
sw $t0, 4($sp)
sw $t2, 8($sp)
# End of creating frame

# Saving frame
# End of saving frame

li $a0,1
jal x

move $a0 $v1
li $v0 1
syscall
li $a0 10
li $v0 11
syscall
li $v0 10
syscall
