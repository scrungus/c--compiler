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
addiu $sp, $sp -12
sw $ra, 4($sp)
li $t1, 12
sw $t1, 0($sp)
# End of creating frame

# Saving frame
# End of saving frame

li $a0,2
jal f
# Restoring frame
lw $ra 4($sp)
# End of restoring frame

addiu $sp, $sp 12
jr $ra

f:
# Creating new frame
addiu $sp, $sp -16
sw $ra, 4($sp)
li $t1, 16
sw $t1, 0($sp)
# End of creating frame

move $t0 $a0
# Saving frame

sw $t0 12($sp)
# End of saving frame

li $a0,2
jal inner
# Restoring frame

lw $t0  16($sp)
lw $ra 4($sp)
# End of restoring frame

addiu $sp, $sp 16
jr $ra
jr $ra
inner:
# Creating new frame
addiu $sp, $sp -16
sw $ra, 4($sp)
li $t1, 16
sw $t1, 0($sp)
# End of creating frame

move $t0 $a0
move $t0,$t0
lw $t1, 4($sp)
add $t2,$t0,$t1
move $v1 $t2
addiu $sp, $sp 16
jr $ra
jr $ra
