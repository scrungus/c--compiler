.globl premain
.text
premain:
# Creating new frame
addiu $sp, $sp -16
li $t1, 16
sw $t1, 0($sp)
li $t2, 1
sw $t2, 8($sp)
li $t3, 0
sw $t3, 4($sp)
# End of creating frame
#saving global frame
li $t0,10

# Saving frame


sw $t0 12($sp)
# End of saving frame
jal main
main:

# Creating new frame
addiu $sp, $sp -16
li $t1, 16
sw $t1, 0($sp)
li $t2, 1
sw $t2, 8($sp)
li $t3, 1
sw $t3, 4($sp)
# End of creating frame
li $t1,20

# Saving frame
sw $t1 12($sp)
# End of saving frame
jal f
f:

# Creating new frame
addiu $sp, $sp -12
li $t1, 12
sw $t1, 0($sp)
li $t2, 0
sw $t2, 8($sp)
li $t3, 2
sw $t3, 4($sp)
# End of creating frame
lw $v1 $20($sp)
addiu $sp, $sp 12
# Restoring frame
lw $t1  12($sp)
# End of saving frame
move $a0 $v1
li $v0 1
syscall
li $a0 10
li $v0 11
syscall
li $v0 10
syscall
